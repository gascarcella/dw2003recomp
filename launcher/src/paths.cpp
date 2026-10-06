#include "paths.h"

#include <SDL3/SDL.h>

namespace dw3 {

static bool is_sep(char c) {
    return c == '/' || c == '\\';
}

static bool has_drive(const std::string &p) {
    return p.size() >= 2 && p[1] == ':' && ((p[0] >= 'A' && p[0] <= 'Z') || (p[0] >= 'a' && p[0] <= 'z'));
}

bool path_is_absolute(const std::string &p) {
    if (!p.empty() && is_sep(p[0])) {
        return true;
    }
    return has_drive(p) && p.size() >= 3 && is_sep(p[2]);
}

std::string path_join(const std::string &dir, const std::string &name) {
    if (dir.empty() || path_is_absolute(name)) {
        return name;
    }
    if (name.empty()) {
        return dir;
    }
    return is_sep(dir.back()) ? dir + name : dir + "/" + name;
}

std::string path_dir(const std::string &p) {
    size_t i = p.find_last_of("/\\");
    if (i == std::string::npos) {
        return "";
    }
    if (i == 0) {
        return p.substr(0, 1);
    }
    if (i == 2 && has_drive(p)) {
        return p.substr(0, 3);
    }
    return p.substr(0, i);
}

std::string path_base(const std::string &p) {
    size_t i = p.find_last_of("/\\");
    return i == std::string::npos ? p : p.substr(i + 1);
}

std::string path_strip_slash(const std::string &p) {
    std::string s = p;
    while (s.size() > 1 && is_sep(s.back()) && !(s.size() == 3 && has_drive(s))) {
        s.pop_back();
    }
    return s;
}

std::string path_resolve(const std::string &base_dir, const std::string &p) {
    return path_is_absolute(p) ? p : path_join(base_dir, p);
}

std::string path_to_url(const std::string &p) {
    static const char HEX[] = "0123456789ABCDEF";
    std::string url = has_drive(p) ? "file:///" : "file://";
    for (unsigned char c : p) {
        if (c == '\\') {
            c = '/';
        }
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || SDL_strchr("/-._~:", c)) {
            url += (char)c;
        } else {
            url += '%';
            url += HEX[c >> 4];
            url += HEX[c & 15];
        }
    }
    return url;
}

bool path_is_file(const std::string &p) {
    SDL_PathInfo info;
    return !p.empty() && SDL_GetPathInfo(p.c_str(), &info) && info.type == SDL_PATHTYPE_FILE;
}

bool path_is_dir(const std::string &p) {
    SDL_PathInfo info;
    return !p.empty() && SDL_GetPathInfo(p.c_str(), &info) && info.type == SDL_PATHTYPE_DIRECTORY;
}

bool path_make_dir(const std::string &p, std::string *err) {
    if (SDL_CreateDirectory(p.c_str())) {
        return true;
    }
    if (err != nullptr) {
        *err = std::string("cannot create ") + p + ": " + SDL_GetError();
    }
    return false;
}

bool file_read(const std::string &path, std::string *out, std::string *err) {
    size_t size = 0;
    void *data = SDL_LoadFile(path.c_str(), &size);
    if (data == nullptr) {
        if (err != nullptr) {
            *err = std::string("cannot read ") + path + ": " + SDL_GetError();
        }
        return false;
    }
    out->assign((const char *)data, size);
    SDL_free(data);
    return true;
}

bool file_write_atomic(const std::string &path, const std::string &data, std::string *err) {
    const std::string tmp = path + ".tmp";
    if (!SDL_SaveFile(tmp.c_str(), data.data(), data.size())) {
        if (err != nullptr) {
            *err = std::string("cannot write ") + tmp + ": " + SDL_GetError();
        }
        SDL_RemovePath(tmp.c_str());
        return false;
    }
    if (!SDL_RenamePath(tmp.c_str(), path.c_str())) {
        if (err != nullptr) {
            *err = std::string("cannot replace ") + path + ": " + SDL_GetError();
        }
        SDL_RemovePath(tmp.c_str());
        return false;
    }
    return true;
}

} // namespace dw3
