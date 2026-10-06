// Paths and files through SDL's filesystem and I/O calls only (no POSIX: the launcher goes to Windows later,
// the Windows track on the project board). Paths are UTF-8; both separators are accepted, '/' is written.
#pragma once

#include <string>

namespace dw3 {

// "/x", "\x", "C:\x", "C:/x" (a drive letter alone, "C:x", counts as relative).
bool path_is_absolute(const std::string &p);
// `dir` + `name` with one separator between them; `name` returned as is when it is absolute or `dir` empty.
std::string path_join(const std::string &dir, const std::string &name);
// Everything before the last separator ("" when there is none; "/" for "/x").
std::string path_dir(const std::string &p);
// The part after the last separator.
std::string path_base(const std::string &p);
// Without a trailing separator (but "/" and "C:\" stay).
std::string path_strip_slash(const std::string &p);
// `p` against `base_dir` when relative.
std::string path_resolve(const std::string &base_dir, const std::string &p);
// A file:// URL for an absolute path (percent-encoded; "C:\x" becomes "file:///C:/x"), for SDL_OpenURL.
std::string path_to_url(const std::string &p);

bool path_is_file(const std::string &p);
bool path_is_dir(const std::string &p);
bool path_make_dir(const std::string &p, std::string *err); // with its parents

// The whole file; false with `err` set when it cannot be read.
bool file_read(const std::string &path, std::string *out, std::string *err);
// Written to `path`.tmp then renamed over `path` (SDL_RenamePath replaces on every system).
bool file_write_atomic(const std::string &path, const std::string &data, std::string *err);

} // namespace dw3
