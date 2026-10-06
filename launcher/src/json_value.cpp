#include "json_value.h"

#include <charconv>
#include <cmath>
#include <cstdio>

extern "C" {
#include "json.h"
}

namespace dw3 {

Json Json::boolean(bool v) {
    Json j;
    j.type_ = Type::Bool;
    j.bool_ = v;
    return j;
}

Json Json::number(double v) {
    Json j;
    j.type_ = Type::Number;
    j.number_ = v;
    return j;
}

Json Json::string(std::string v) {
    Json j;
    j.type_ = Type::String;
    j.string_ = std::move(v);
    return j;
}

Json Json::array() {
    Json j;
    j.type_ = Type::Array;
    return j;
}

Json Json::object() {
    Json j;
    j.type_ = Type::Object;
    return j;
}

static Json from_port(const PortJson *v) {
    switch (v->type) {
    case PORT_JSON_NULL:
        return Json();
    case PORT_JSON_BOOL:
        return Json::boolean(v->boolean != 0);
    case PORT_JSON_NUMBER:
        return Json::number(v->number);
    case PORT_JSON_STRING:
        return Json::string(v->string);
    case PORT_JSON_ARRAY: {
        Json a = Json::array();
        for (size_t i = 0; i < v->count; i++) {
            a.push(from_port(&v->items[i]));
        }
        return a;
    }
    case PORT_JSON_OBJECT: {
        Json o = Json::object();
        for (size_t i = 0; i < v->count; i++) {
            o.set(v->keys[i], from_port(&v->items[i]));
        }
        return o;
    }
    }
    return Json();
}

Json Json::parse(const std::string &text, std::string *err) {
    char msg[256] = "";
    PortJson *root = port_json_parse(text.data(), text.size(), msg, sizeof(msg));
    if (root == nullptr) {
        if (err != nullptr) {
            *err = msg;
        }
        return Json();
    }
    Json j = from_port(root);
    port_json_free(root);
    if (err != nullptr) {
        err->clear();
    }
    return j;
}

int Json::as_int(int fallback, int lo, int hi) const {
    if (type_ != Type::Number || number_ != std::floor(number_) || number_ < lo || number_ > hi) {
        return fallback;
    }
    return (int)number_;
}

const Json *Json::find(const std::string &key) const {
    if (type_ != Type::Object) {
        return nullptr;
    }
    for (const auto &m : members_) {
        if (m.first == key) {
            return &m.second;
        }
    }
    return nullptr;
}

Json *Json::find(const std::string &key) {
    return const_cast<Json *>(static_cast<const Json *>(this)->find(key));
}

Json &Json::member(const std::string &key) {
    if (Json *m = find(key)) {
        return *m;
    }
    set(key, Json());
    return members_.back().second;
}

void Json::set(const std::string &key, Json value) {
    if (type_ != Type::Object) {
        *this = object();
    }
    if (Json *m = find(key)) {
        *m = std::move(value);
        return;
    }
    members_.emplace_back(key, std::move(value));
}

bool Json::erase(const std::string &key) {
    for (auto it = members_.begin(); it != members_.end(); ++it) {
        if (it->first == key) {
            members_.erase(it);
            return true;
        }
    }
    return false;
}

void Json::push(Json value) {
    if (type_ != Type::Array) {
        *this = array();
    }
    items_.push_back(std::move(value));
}

bool Json::operator==(const Json &o) const {
    if (type_ != o.type_) {
        return false;
    }
    switch (type_) {
    case Type::Null:
        return true;
    case Type::Bool:
        return bool_ == o.bool_;
    case Type::Number:
        return number_ == o.number_;
    case Type::String:
        return string_ == o.string_;
    case Type::Array:
        return items_ == o.items_;
    case Type::Object:
        return members_ == o.members_;
    }
    return false;
}

static void dump_string(std::string &out, const std::string &s) {
    out += '"';
    for (unsigned char c : s) {
        switch (c) {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            if (c < 0x20) {
                char buf[8];
                std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                out += buf;
            } else {
                out += (char)c; // UTF-8 passes through
            }
        }
    }
    out += '"';
}

static void dump_number(std::string &out, double v) {
    if (!std::isfinite(v)) {
        out += "null"; // JSON has no NaN or infinity
        return;
    }
    char buf[32];
    auto r = std::to_chars(buf, buf + sizeof(buf), v); // the shortest text that reads back as v
    out.append(buf, r.ptr);
}

void Json::dump_to(std::string &out, int depth) const {
    const std::string indent((size_t)(depth + 1) * 2, ' ');
    switch (type_) {
    case Type::Null:
        out += "null";
        break;
    case Type::Bool:
        out += bool_ ? "true" : "false";
        break;
    case Type::Number:
        dump_number(out, number_);
        break;
    case Type::String:
        dump_string(out, string_);
        break;
    case Type::Array:
        if (items_.empty()) {
            out += "[]";
            break;
        }
        out += "[\n";
        for (size_t i = 0; i < items_.size(); i++) {
            out += indent;
            items_[i].dump_to(out, depth + 1);
            out += i + 1 < items_.size() ? ",\n" : "\n";
        }
        out.append((size_t)depth * 2, ' ');
        out += ']';
        break;
    case Type::Object:
        if (members_.empty()) {
            out += "{}";
            break;
        }
        out += "{\n";
        for (size_t i = 0; i < members_.size(); i++) {
            out += indent;
            dump_string(out, members_[i].first);
            out += ": ";
            members_[i].second.dump_to(out, depth + 1);
            out += i + 1 < members_.size() ? ",\n" : "\n";
        }
        out.append((size_t)depth * 2, ' ');
        out += '}';
        break;
    }
}

std::string Json::dump() const {
    std::string out;
    dump_to(out, 0);
    out += '\n';
    return out;
}

} // namespace dw3
