#include "disc.h"

#include <vector>

#include "paths.h"

extern "C" {
#include "sha1.h"
}

namespace dw3 {

static bool ends_with_ci(const std::string &s, const char *suffix) {
    size_t n = SDL_strlen(suffix);
    return s.size() >= n && SDL_strcasecmp(s.c_str() + s.size() - n, suffix) == 0;
}

bool disc_bin_path(const std::string &path, std::string *bin, std::string *err) {
    if (!ends_with_ci(path, ".cue")) {
        *bin = path;
        return true;
    }
    std::string text;
    if (!file_read(path, &text, err)) {
        return false;
    }
    size_t pos = 0;
    while (pos < text.size()) {
        size_t eol = text.find_first_of("\r\n", pos);
        std::string line = text.substr(pos, eol == std::string::npos ? std::string::npos : eol - pos);
        pos = eol == std::string::npos ? text.size() : eol + 1;
        size_t p = line.find_first_not_of(" \t");
        if (p == std::string::npos || line.compare(p, 4, "FILE") != 0 || p + 4 >= line.size() ||
            (line[p + 4] != ' ' && line[p + 4] != '\t')) {
            continue;
        }
        p = line.find_first_not_of(" \t", p + 4);
        std::string name;
        if (p != std::string::npos && line[p] == '"') {
            size_t end = line.find('"', p + 1);
            if (end != std::string::npos) {
                name = line.substr(p + 1, end - p - 1);
            }
        } else if (p != std::string::npos) {
            name = line.substr(p, line.find_first_of(" \t", p) - p);
        }
        if (name.empty()) {
            *err = path + ": cannot read its FILE line";
            return false;
        }
        *bin = path_resolve(path_dir(path), name);
        return true;
    }
    *err = path + ": no FILE line (is it a .cue sheet?)";
    return false;
}

int64_t disc_file_size(const std::string &path) {
    SDL_PathInfo info;
    if (!SDL_GetPathInfo(path.c_str(), &info) || info.type != SDL_PATHTYPE_FILE) {
        return -1;
    }
    return (int64_t)info.size;
}

DiscCheck::~DiscCheck() {
    cancel();
    join();
}

void DiscCheck::start(const std::string &path) {
    cancel();
    join();
    path_ = path;
    cancel_ = false;
    done_ = false;
    read_ = 0;
    total_ = 1;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sha1_.clear();
        message_.clear();
        passed_ = false;
    }
    state_ = State::Running;
    thread_ = SDL_CreateThread(thread_main, "disc-check", this);
    if (thread_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        message_ = std::string("cannot start the check: ") + SDL_GetError();
        state_ = State::Failed;
    }
}

void DiscCheck::cancel() {
    cancel_ = true;
}

void DiscCheck::join() {
    if (thread_ != nullptr) {
        SDL_WaitThread(thread_, nullptr);
        thread_ = nullptr;
    }
}

DiscCheck::State DiscCheck::poll() {
    if (state_ == State::Running && done_) {
        join();
        std::lock_guard<std::mutex> lock(mutex_);
        state_ = cancel_ ? State::Cancelled : passed_ ? State::Passed : State::Failed;
    }
    return state_;
}

float DiscCheck::progress() const {
    uint64_t total = total_;
    return total == 0 ? 0.0f : (float)((double)read_ / (double)total);
}

std::string DiscCheck::sha1() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return sha1_;
}

std::string DiscCheck::message() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return message_;
}

int DiscCheck::thread_main(void *self) {
    static_cast<DiscCheck *>(self)->run();
    return 0;
}

void DiscCheck::run() {
    std::string bin, err, sha;
    bool passed = false;
    if (!disc_bin_path(path_, &bin, &err)) {
        // err set
    } else if (SDL_IOStream *io = SDL_IOFromFile(bin.c_str(), "rb")) {
        Sint64 size = SDL_GetIOSize(io);
        total_ = size > 0 ? (uint64_t)size : 1;
        std::vector<uint8_t> buf(4 << 20);
        PortSha1 c;
        port_sha1_init(&c);
        while (!cancel_) {
            size_t n = SDL_ReadIO(io, buf.data(), buf.size());
            if (n == 0) {
                if (SDL_GetIOStatus(io) != SDL_IO_STATUS_EOF) {
                    err = "reading " + bin + ": " + SDL_GetError();
                }
                break;
            }
            port_sha1_update(&c, buf.data(), n);
            read_ += n;
        }
        SDL_CloseIO(io);
        if (!cancel_ && err.empty()) {
            uint8_t digest[20];
            char hex[41];
            port_sha1_final(&c, digest);
            port_sha1_hex(digest, hex);
            sha = hex;
            passed = sha == DISC_SHA1;
            if (!passed) {
                err = bin + " is not the unpatched European disc (SLES-03936): its SHA-1 is " + sha + ", expected " +
                      DISC_SHA1 + " (a Redump dump of the EU disc, as one .bin with its .cue)";
            }
        }
    } else {
        err = "cannot open " + bin + ": " + SDL_GetError();
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sha1_ = sha;
        message_ = err;
        passed_ = passed;
    }
    done_ = true;
}

} // namespace dw3
