/* The settings file (settings.h; `--config FILE`): schema 1 of docs/LAUNCHER.md "Settings file", the contract with
 * the launcher. Every key but `schema` is optional; an absent key keeps its default:
 *
 *   { "schema": 1,
 *     "disc":     { "path": "dw2003.cue", "sha1": "457cb233..." },          none / none
 *     "video":    { "window": true, "scale": 2, "fullscreen": false, "refresh": 50, "renderer": "software",
 *                   "internal_scale": 1 },
 *     "audio":    { "mute": false },
 *     "memcard1": "card1.mcd", "memcard2": "card2.mcd",                    null: no card in that slot
 *     "watchdog": 0,                                                       seconds; 0 off
 *     "input":    { "keyboard": {...}, "gamepad": {...}, "hotkeys": {...} },   input.c (port_input_settings)
 *     "mods":     { "<id>": { "enabled": false, "<option>": value } },          mods.c (port_mods_settings)
 *     "launcher": { ... } }                                                the launcher's own; never read
 *
 * Paths are relative to the file's directory (or absolute). A value of the wrong type or out of range ends the run
 * with status 64, as a bad option does; an unknown key is logged and ignored, so an older game runs a newer launcher's
 * file. The values are applied by main.c, under the command line's options. */
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "platform.h"
#include "port_harness.h"
#include "port_runtime.h"
#include "settings.h"

PortSettings port_settings;
int port_settings_loaded;

void port_settings_fail(const char *key, const char *fmt, ...) {
    va_list ap;
    fprintf(stderr, "port: settings %s: %s%s", port_settings.file, key != NULL ? key : "", key != NULL ? ": " : "");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    exit(64);
}

static char *settings_strdup(const char *s) {
    char *d = strdup(s);
    if (d == NULL) {
        port_fatal("settings: out of memory");
    }
    return d;
}

/* `dir` + "/" + `path`, or `path` when it is absolute (port_path_is_absolute: "/x" on both, "C:\x" on Windows). */
static char *settings_join(const char *dir, const char *path) {
    char *out;
    if (port_path_is_absolute(path) || dir == NULL) {
        return settings_strdup(path);
    }
    out = malloc(strlen(dir) + strlen(path) + 2);
    if (out == NULL) {
        port_fatal("settings: out of memory");
    }
    sprintf(out, "%s%s%s", dir, dir[0] != '\0' && port_path_is_sep(dir[strlen(dir) - 1]) ? "" : "/", path);
    return out;
}

char *port_settings_abspath(const char *path) {
    char cwd[4096];
    if (port_path_is_absolute(path)) {
        return settings_strdup(path);
    }
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        port_fatal("settings: getcwd: %s", strerror(errno));
    }
    return settings_join(cwd, path);
}

/* Logs every key of `obj` that is not in `known` (NULL-terminated): ignored. */
static void settings_unknown(const PortJson *obj, const char *where, const char *const *known) {
    size_t i;
    for (i = 0; i < obj->count; i++) {
        const char *const *k;
        for (k = known; *k != NULL && strcmp(*k, obj->keys[i]) != 0; k++) {
        }
        if (*k == NULL) {
            port_log("settings: %s%s%s: unknown key, ignored", where, where[0] ? "." : "", obj->keys[i]);
        }
    }
}

static const char *settings_article(PortJsonType type) {
    return type == PORT_JSON_ARRAY || type == PORT_JSON_OBJECT ? "an" : "a";
}

/* The member `key` of `obj` checked to be of `type` (NULL when absent). `where` names `obj` in messages. */
static const PortJson *settings_member(const PortJson *obj, const char *where, const char *key, PortJsonType type) {
    const PortJson *v = port_json_get(obj, key);
    if (v != NULL && v->type != type) {
        char name[128];
        snprintf(name, sizeof(name), "%s%s%s", where, where[0] ? "." : "", key);
        port_settings_fail(name, "%s %s, not %s %s", settings_article(type), port_json_type_name(type),
                      settings_article(v->type), port_json_type_name(v->type));
    }
    return v;
}

static void settings_bool(const PortJson *obj, const char *where, const char *key, int *out) {
    const PortJson *v = settings_member(obj, where, key, PORT_JSON_BOOL);
    if (v != NULL) {
        *out = v->boolean;
    }
}

static void settings_int(const PortJson *obj, const char *where, const char *key, int lo, int hi, int *out) {
    const PortJson *v = settings_member(obj, where, key, PORT_JSON_NUMBER);
    if (v != NULL) {
        if (v->number != (double)(long)v->number || v->number < lo || v->number > hi) {
            char name[128];
            snprintf(name, sizeof(name), "%s%s%s", where, where[0] ? "." : "", key);
            port_settings_fail(name, "an integer from %d to %d, not %g", lo, hi, v->number);
        }
        *out = (int)v->number;
    }
}

/* A memory card: a path (relative to the file's directory), or null for no card. */
static void settings_memcard(const PortJson *root, const char *key, int slot) {
    const PortJson *v = port_json_get(root, key);
    if (v == NULL) {
        return;
    }
    free(port_settings.memcard[slot]);
    port_settings.memcard[slot] = NULL;
    if (v->type == PORT_JSON_NULL) {
        return;
    }
    if (v->type != PORT_JSON_STRING || v->string[0] == '\0') {
        port_settings_fail(key, "a path to a .mcd image (relative to the settings file), or null for no card");
    }
    port_settings.memcard[slot] = settings_join(port_settings.dir, v->string);
}

void port_settings_load(const char *path) {
    static const char *const top_keys[] = { "schema", "disc", "video", "audio", "memcard1", "memcard2", "watchdog",
                                            "input", "mods", "launcher", NULL };
    static const char *const disc_keys[] = { "path", "sha1", NULL };
    static const char *const video_keys[] = { "window", "scale", "fullscreen", "refresh", "renderer", "internal_scale",
                                              NULL };
    static const char *const audio_keys[] = { "mute", NULL };
    PortSettings *s = &port_settings;
    const PortJson *root, *v, *obj;
    char err[256], *slash;
    FILE *f;
    char *text;
    long size;

    s->file = port_settings_abspath(path);
    s->dir = settings_strdup(s->file);
    slash = (char *)port_path_last_sep(s->dir);
    if (slash == NULL) {
        port_settings_fail(NULL, "not a path: %s", s->file);
    }
    slash[slash == s->dir ? 1 : 0] = '\0';
    s->window = 1;
    s->scale = 2;
    s->refresh = 50;
    s->internal_scale = 1;
    s->memcard[0] = settings_join(s->dir, "card1.mcd");
    s->memcard[1] = settings_join(s->dir, "card2.mcd");

    f = fopen(path, "rb");
    if (f == NULL || fseek(f, 0, SEEK_END) != 0 || (size = ftell(f)) < 0 || fseek(f, 0, SEEK_SET) != 0) {
        port_settings_fail(NULL, "cannot read: %s", strerror(errno));
    }
    text = malloc((size_t)size + 1);
    if (text == NULL || fread(text, 1, (size_t)size, f) != (size_t)size) {
        port_settings_fail(NULL, "cannot read");
    }
    fclose(f);
    s->root = port_json_parse(text, (size_t)size, err, sizeof(err));
    free(text);
    if (s->root == NULL) {
        port_settings_fail(NULL, "not JSON: %s", err);
    }
    root = s->root;
    if (root->type != PORT_JSON_OBJECT) {
        port_settings_fail(NULL, "the settings are an object, not %s %s", settings_article(root->type),
                      port_json_type_name(root->type));
    }
    v = settings_member(root, "", "schema", PORT_JSON_NUMBER);
    if (v == NULL) {
        port_settings_fail("schema", "missing (this game reads schema %d)", PORT_SETTINGS_SCHEMA);
    }
    if (v->number != PORT_SETTINGS_SCHEMA) {
        port_settings_fail("schema", "%g, but this game reads schema %d%s", v->number, PORT_SETTINGS_SCHEMA,
                      v->number > PORT_SETTINGS_SCHEMA ? " (the file is a newer launcher's)" : "");
    }
    settings_unknown(root, "", top_keys);

    if ((obj = settings_member(root, "", "disc", PORT_JSON_OBJECT)) != NULL) {
        settings_unknown(obj, "disc", disc_keys);
        if ((v = settings_member(obj, "disc", "path", PORT_JSON_STRING)) != NULL && v->string[0] != '\0') {
            s->disc = settings_join(s->dir, v->string);
        }
        if ((v = settings_member(obj, "disc", "sha1", PORT_JSON_STRING)) != NULL && v->string[0] != '\0') {
            s->disc_sha1 = settings_strdup(v->string);
        }
    }
    if ((obj = settings_member(root, "", "video", PORT_JSON_OBJECT)) != NULL) {
        settings_unknown(obj, "video", video_keys);
        settings_bool(obj, "video", "window", &s->window);
        settings_int(obj, "video", "scale", 1, 16, &s->scale);
        settings_bool(obj, "video", "fullscreen", &s->fullscreen);
        settings_int(obj, "video", "refresh", 50, 60, &s->refresh);
        if (s->refresh != 50 && s->refresh != 60) {
            port_settings_fail("video.refresh", "50 (PAL) or 60, not %d", s->refresh);
        }
        if ((v = settings_member(obj, "video", "renderer", PORT_JSON_STRING)) != NULL) {
            if (strcmp(v->string, "software") != 0 && strcmp(v->string, "gpu") != 0) {
                port_settings_fail("video.renderer", "\"software\" or \"gpu\", not \"%s\"", v->string);
            }
            s->gpu = v->string[0] == 'g';
        }
        settings_int(obj, "video", "internal_scale", 1, 8, &s->internal_scale);
    }
    if ((obj = settings_member(root, "", "audio", PORT_JSON_OBJECT)) != NULL) {
        settings_unknown(obj, "audio", audio_keys);
        settings_bool(obj, "audio", "mute", &s->mute);
    }
    settings_memcard(root, "memcard1", 0);
    settings_memcard(root, "memcard2", 1);
    settings_int(root, "", "watchdog", 0, 3600, &s->watchdog);
    s->input = settings_member(root, "", "input", PORT_JSON_OBJECT);
    s->mods = settings_member(root, "", "mods", PORT_JSON_OBJECT);
    s->launcher = settings_member(root, "", "launcher", PORT_JSON_OBJECT);
    port_settings_loaded = 1;
    port_log("settings: %s", s->file);
}

static void settings_print_path(FILE *f, const char *path) {
    if (path == NULL) {
        fputs("null", f);
    } else {
        port_json_write_string(f, path);
    }
}

void port_settings_print(FILE *f, const PortSettings *s) {
    fprintf(f, "{\n  \"schema\": %d,\n  \"disc\": {\n    \"path\": ", PORT_SETTINGS_SCHEMA);
    port_json_write_string(f, s->disc != NULL ? s->disc : "");
    fputs(",\n    \"sha1\": ", f);
    port_json_write_string(f, s->disc_sha1 != NULL ? s->disc_sha1 : "");
    fprintf(f, "\n  },\n  \"video\": {\n    \"window\": %s,\n    \"scale\": %d,\n    \"fullscreen\": %s,\n"
               "    \"refresh\": %d,\n    \"renderer\": \"%s\",\n    \"internal_scale\": %d\n  },\n"
               "  \"audio\": {\n    \"mute\": %s\n  },\n"
               "  \"memcard1\": ",
            s->window ? "true" : "false", s->scale, s->fullscreen ? "true" : "false", s->refresh,
            s->gpu ? "gpu" : "software", s->internal_scale, s->mute ? "true" : "false");
    settings_print_path(f, s->memcard[0]);
    fputs(",\n  \"memcard2\": ", f);
    settings_print_path(f, s->memcard[1]);
    fprintf(f, ",\n  \"watchdog\": %d", s->watchdog);
    if (s->input != NULL) {
        fputs(",\n  \"input\": ", f);
        port_json_write(f, s->input, 2, 1);
    }
    fputs(",\n  \"mods\": ", f);
    port_mods_print(f, 1);
    if (s->launcher != NULL) {
        fputs(",\n  \"launcher\": ", f);
        port_json_write(f, s->launcher, 2, 1);
    }
    fputs("\n}\n", f);
}
