/* A small, strict JSON reader (json.h): a recursive-descent parser over the whole text, building a tree of PortJson
 * values. Errors carry the line and column of the offending byte. */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"

typedef struct PortJsonParser {
    const char *text;
    size_t length;
    size_t pos;
    char *err;
    size_t err_size;
    int failed;
} PortJsonParser;

static void port_json_error(PortJsonParser *p, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
static void port_json_error(PortJsonParser *p, const char *fmt, ...) {
    va_list ap;
    size_t i, line = 1, column = 1;
    int n;
    if (p->failed) {
        return; /* keep the first error */
    }
    p->failed = 1;
    for (i = 0; i < p->pos && i < p->length; i++) {
        if (p->text[i] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
    }
    if (p->err_size == 0) {
        return;
    }
    n = snprintf(p->err, p->err_size, "line %zu column %zu: ", line, column);
    if (n >= 0 && (size_t)n < p->err_size) {
        va_start(ap, fmt);
        vsnprintf(p->err + n, p->err_size - (size_t)n, fmt, ap);
        va_end(ap);
    }
}

static int port_json_peek(const PortJsonParser *p) {
    return p->pos < p->length ? (unsigned char)p->text[p->pos] : -1;
}

static void port_json_skip_space(PortJsonParser *p) {
    while (p->pos < p->length) {
        char c = p->text[p->pos];
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            break;
        }
        p->pos++;
    }
}

static void *port_json_alloc(PortJsonParser *p, void *old, size_t size) {
    void *mem = realloc(old, size != 0 ? size : 1);
    if (mem == NULL) {
        port_json_error(p, "out of memory");
    }
    return mem;
}

/* A growing byte buffer for strings. */
typedef struct PortJsonBuf {
    char *data;
    size_t n, cap;
} PortJsonBuf;

static int port_json_buf_put(PortJsonParser *p, PortJsonBuf *b, char c) {
    if (b->n + 1 >= b->cap) {
        size_t cap = b->cap ? b->cap * 2 : 32;
        char *data = port_json_alloc(p, b->data, cap);
        if (data == NULL) {
            return 0;
        }
        b->data = data;
        b->cap = cap;
    }
    b->data[b->n++] = c;
    b->data[b->n] = '\0';
    return 1;
}

static int port_json_buf_put_utf8(PortJsonParser *p, PortJsonBuf *b, unsigned long cp) {
    if (cp < 0x80) {
        return port_json_buf_put(p, b, (char)cp);
    }
    if (cp < 0x800) {
        return port_json_buf_put(p, b, (char)(0xC0 | (cp >> 6))) && port_json_buf_put(p, b, (char)(0x80 | (cp & 0x3F)));
    }
    if (cp < 0x10000) {
        return port_json_buf_put(p, b, (char)(0xE0 | (cp >> 12))) &&
               port_json_buf_put(p, b, (char)(0x80 | ((cp >> 6) & 0x3F))) &&
               port_json_buf_put(p, b, (char)(0x80 | (cp & 0x3F)));
    }
    return port_json_buf_put(p, b, (char)(0xF0 | (cp >> 18))) &&
           port_json_buf_put(p, b, (char)(0x80 | ((cp >> 12) & 0x3F))) &&
           port_json_buf_put(p, b, (char)(0x80 | ((cp >> 6) & 0x3F))) &&
           port_json_buf_put(p, b, (char)(0x80 | (cp & 0x3F)));
}

/* Four hex digits after "\u"; -1 on error. */
static long port_json_hex4(PortJsonParser *p) {
    long v = 0;
    int i;
    for (i = 0; i < 4; i++) {
        int c = port_json_peek(p);
        int d;
        if (c >= '0' && c <= '9') {
            d = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            d = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            d = c - 'A' + 10;
        } else {
            port_json_error(p, "bad \\u escape (four hex digits expected)");
            return -1;
        }
        v = v * 16 + d;
        p->pos++;
    }
    return v;
}

/* A string (the opening quote at pos); returns it, malloc'd, or NULL. */
static char *port_json_parse_string(PortJsonParser *p) {
    PortJsonBuf b = { NULL, 0, 0 };
    p->pos++; /* the opening quote */
    if (!port_json_buf_put(p, &b, '\0')) {
        return NULL;
    }
    b.n = 0;
    for (;;) {
        int c = port_json_peek(p);
        if (c < 0) {
            port_json_error(p, "unterminated string");
            break;
        }
        if (c == '"') {
            p->pos++;
            return b.data;
        }
        if (c < 0x20) {
            port_json_error(p, "control character 0x%02X in a string", c);
            break;
        }
        if (c != '\\') {
            if (!port_json_buf_put(p, &b, (char)c)) {
                break;
            }
            p->pos++;
            continue;
        }
        p->pos++;
        c = port_json_peek(p);
        p->pos++;
        switch (c) {
        case '"':
        case '\\':
        case '/':
            c = port_json_buf_put(p, &b, (char)c);
            break;
        case 'b':
            c = port_json_buf_put(p, &b, '\b');
            break;
        case 'f':
            c = port_json_buf_put(p, &b, '\f');
            break;
        case 'n':
            c = port_json_buf_put(p, &b, '\n');
            break;
        case 'r':
            c = port_json_buf_put(p, &b, '\r');
            break;
        case 't':
            c = port_json_buf_put(p, &b, '\t');
            break;
        case 'u': {
            long cp = port_json_hex4(p), lo;
            if (cp < 0) {
                c = 0;
                break;
            }
            if (cp >= 0xD800 && cp < 0xDC00) {
                /* a high surrogate: a low one must follow */
                if (port_json_peek(p) != '\\' || p->pos + 1 >= p->length || p->text[p->pos + 1] != 'u') {
                    port_json_error(p, "unpaired surrogate \\u%04lX", cp);
                    c = 0;
                    break;
                }
                p->pos += 2;
                lo = port_json_hex4(p);
                if (lo < 0) {
                    c = 0;
                    break;
                }
                if (lo < 0xDC00 || lo >= 0xE000) {
                    port_json_error(p, "unpaired surrogate \\u%04lX", cp);
                    c = 0;
                    break;
                }
                cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
            } else if (cp >= 0xDC00 && cp < 0xE000) {
                port_json_error(p, "unpaired surrogate \\u%04lX", cp);
                c = 0;
                break;
            }
            c = port_json_buf_put_utf8(p, &b, (unsigned long)cp);
            break;
        }
        default:
            p->pos--;
            port_json_error(p, "bad escape in a string");
            c = 0;
            break;
        }
        if (!c) {
            break;
        }
    }
    free(b.data);
    return NULL;
}

/* A number in JSON's grammar: -?(0|[1-9][0-9]*)(.[0-9]+)?([eE][+-]?[0-9]+)? */
static int port_json_parse_number(PortJsonParser *p, double *out) {
    size_t start = p->pos;
    char buf[64];
    char *end;
    int c = port_json_peek(p);
    if (c == '-') {
        p->pos++;
        c = port_json_peek(p);
    }
    if (c == '0') {
        p->pos++;
    } else if (c >= '1' && c <= '9') {
        while ((c = port_json_peek(p)) >= '0' && c <= '9') {
            p->pos++;
        }
    } else {
        port_json_error(p, "bad number");
        return 0;
    }
    if (port_json_peek(p) == '.') {
        p->pos++;
        if (!((c = port_json_peek(p)) >= '0' && c <= '9')) {
            port_json_error(p, "bad number (a digit must follow the point)");
            return 0;
        }
        while ((c = port_json_peek(p)) >= '0' && c <= '9') {
            p->pos++;
        }
    }
    c = port_json_peek(p);
    if (c == 'e' || c == 'E') {
        p->pos++;
        c = port_json_peek(p);
        if (c == '+' || c == '-') {
            p->pos++;
        }
        if (!((c = port_json_peek(p)) >= '0' && c <= '9')) {
            port_json_error(p, "bad number (a digit must follow the exponent)");
            return 0;
        }
        while ((c = port_json_peek(p)) >= '0' && c <= '9') {
            p->pos++;
        }
    }
    if (p->pos - start >= sizeof(buf)) {
        port_json_error(p, "number too long");
        return 0;
    }
    memcpy(buf, p->text + start, p->pos - start);
    buf[p->pos - start] = '\0';
    *out = strtod(buf, &end); /* the grammar above is a subset of strtod's (the C locale: the program sets none) */
    return 1;
}

static int port_json_literal(PortJsonParser *p, const char *word) {
    size_t n = strlen(word);
    if (p->length - p->pos < n || memcmp(p->text + p->pos, word, n) != 0) {
        port_json_error(p, "unexpected character");
        return 0;
    }
    p->pos += n;
    return 1;
}

static void port_json_free_children(PortJson *v);

/* One value (whitespace before it already skipped) into *v. */
static int port_json_parse_value(PortJsonParser *p, PortJson *v, int depth) {
    int c = port_json_peek(p);
    memset(v, 0, sizeof(*v));
    if (c < 0) {
        port_json_error(p, "unexpected end of the text");
        return 0;
    }
    if (c == '{' || c == '[') {
        int object = c == '{';
        size_t cap = 0;
        if (depth >= PORT_JSON_MAX_DEPTH) {
            port_json_error(p, "nested deeper than %d", PORT_JSON_MAX_DEPTH);
            return 0;
        }
        v->type = object ? PORT_JSON_OBJECT : PORT_JSON_ARRAY;
        p->pos++;
        port_json_skip_space(p);
        if (port_json_peek(p) == (object ? '}' : ']')) {
            p->pos++;
            return 1;
        }
        for (;;) {
            char *key = NULL;
            if (object) {
                size_t i;
                if (port_json_peek(p) != '"') {
                    port_json_error(p, "a key (a string) expected");
                    return 0;
                }
                key = port_json_parse_string(p);
                if (key == NULL) {
                    return 0;
                }
                for (i = 0; i < v->count; i++) {
                    if (strcmp(v->keys[i], key) == 0) {
                        port_json_error(p, "duplicate key \"%s\"", key);
                        free(key);
                        return 0;
                    }
                }
                port_json_skip_space(p);
                if (port_json_peek(p) != ':') {
                    port_json_error(p, "':' expected after a key");
                    free(key);
                    return 0;
                }
                p->pos++;
                port_json_skip_space(p);
            }
            if (v->count == cap) {
                PortJson *items;
                cap = cap ? cap * 2 : 8;
                items = port_json_alloc(p, v->items, cap * sizeof(*items));
                if (items == NULL) {
                    free(key);
                    return 0;
                }
                v->items = items;
                if (object) {
                    char **keys = port_json_alloc(p, v->keys, cap * sizeof(*keys));
                    if (keys == NULL) {
                        free(key);
                        return 0;
                    }
                    v->keys = keys;
                }
            }
            if (object) {
                v->keys[v->count] = key;
            }
            if (!port_json_parse_value(p, &v->items[v->count], depth + 1)) {
                /* count the item so port_json_free releases what it holds and its key */
                v->count++;
                return 0;
            }
            v->count++;
            port_json_skip_space(p);
            c = port_json_peek(p);
            if (c == ',') {
                p->pos++;
                port_json_skip_space(p);
                continue;
            }
            if (c == (object ? '}' : ']')) {
                p->pos++;
                return 1;
            }
            port_json_error(p, object ? "',' or '}' expected" : "',' or ']' expected");
            return 0;
        }
    }
    if (c == '"') {
        v->type = PORT_JSON_STRING;
        v->string = port_json_parse_string(p);
        return v->string != NULL;
    }
    if (c == '-' || (c >= '0' && c <= '9')) {
        v->type = PORT_JSON_NUMBER;
        return port_json_parse_number(p, &v->number);
    }
    if (c == 't' || c == 'f') {
        v->type = PORT_JSON_BOOL;
        v->boolean = c == 't';
        return port_json_literal(p, c == 't' ? "true" : "false");
    }
    if (c == 'n') {
        v->type = PORT_JSON_NULL;
        return port_json_literal(p, "null");
    }
    port_json_error(p, "unexpected character '%c'", c >= 0x20 && c < 0x7F ? c : '?');
    return 0;
}

PortJson *port_json_parse(const char *text, size_t length, char *err, size_t err_size) {
    PortJsonParser p;
    PortJson *root;
    memset(&p, 0, sizeof(p));
    p.text = text;
    p.length = length;
    p.err = err;
    p.err_size = err_size;
    if (err_size > 0) {
        err[0] = '\0';
    }
    root = malloc(sizeof(*root));
    if (root == NULL) {
        port_json_error(&p, "out of memory");
        return NULL;
    }
    port_json_skip_space(&p);
    if (port_json_parse_value(&p, root, 0)) {
        port_json_skip_space(&p);
        if (p.pos < p.length) {
            port_json_error(&p, "text after the value");
        }
    }
    if (p.failed) {
        port_json_free(root);
        return NULL;
    }
    return root;
}

const PortJson *port_json_get(const PortJson *obj, const char *key) {
    size_t i;
    if (obj == NULL || obj->type != PORT_JSON_OBJECT) {
        return NULL;
    }
    for (i = 0; i < obj->count; i++) {
        if (strcmp(obj->keys[i], key) == 0) {
            return &obj->items[i];
        }
    }
    return NULL;
}

const char *port_json_type_name(PortJsonType type) {
    switch (type) {
    case PORT_JSON_NULL:
        return "null";
    case PORT_JSON_BOOL:
        return "boolean";
    case PORT_JSON_NUMBER:
        return "number";
    case PORT_JSON_STRING:
        return "string";
    case PORT_JSON_ARRAY:
        return "array";
    case PORT_JSON_OBJECT:
        return "object";
    }
    return "?";
}

static void port_json_free_children(PortJson *v) {
    size_t i;
    if (v->type == PORT_JSON_STRING) {
        free(v->string);
    } else if (v->type == PORT_JSON_ARRAY || v->type == PORT_JSON_OBJECT) {
        for (i = 0; i < v->count; i++) {
            port_json_free_children(&v->items[i]);
            if (v->keys != NULL) {
                free(v->keys[i]);
            }
        }
        free(v->items);
        free(v->keys);
    }
}

void port_json_free(PortJson *value) {
    if (value != NULL) {
        port_json_free_children(value);
        free(value);
    }
}

/* ---- The writer (settings.c's --print-settings): strings escaped as RFC 8259 requires, numbers that are integers
 * printed as integers, objects and arrays one item per line at `indent` spaces a level (indent 0: on one line). */
void port_json_write_string(FILE *f, const char *s) {
    fputc('"', f);
    for (; *s != '\0'; s++) {
        unsigned char c = (unsigned char)*s;
        if (c == '"' || c == '\\') {
            fputc('\\', f);
            fputc(c, f);
        } else if (c == '\n') {
            fputs("\\n", f);
        } else if (c == '\t') {
            fputs("\\t", f);
        } else if (c == '\r') {
            fputs("\\r", f);
        } else if (c < 0x20) {
            fprintf(f, "\\u%04x", c);
        } else {
            fputc(c, f);
        }
    }
    fputc('"', f);
}

void port_json_write(FILE *f, const PortJson *v, int indent, int level) {
    size_t i;
    switch (v->type) {
    case PORT_JSON_NULL:
        fputs("null", f);
        break;
    case PORT_JSON_BOOL:
        fputs(v->boolean ? "true" : "false", f);
        break;
    case PORT_JSON_NUMBER:
        if (v->number == (double)(long long)v->number && v->number > -9007199254740992.0 &&
            v->number < 9007199254740992.0) {
            fprintf(f, "%lld", (long long)v->number);
        } else {
            fprintf(f, "%.17g", v->number);
        }
        break;
    case PORT_JSON_STRING:
        port_json_write_string(f, v->string);
        break;
    case PORT_JSON_ARRAY:
    case PORT_JSON_OBJECT:
        if (v->count == 0) {
            fputs(v->type == PORT_JSON_ARRAY ? "[]" : "{}", f);
            break;
        }
        fputc(v->type == PORT_JSON_ARRAY ? '[' : '{', f);
        for (i = 0; i < v->count; i++) {
            if (indent == 0) {
                fputs(i ? ", " : "", f); /* one line */
            } else {
                fprintf(f, "%s\n%*s", i ? "," : "", indent * (level + 1), "");
            }
            if (v->type == PORT_JSON_OBJECT) {
                port_json_write_string(f, v->keys[i]);
                fputs(": ", f);
            }
            port_json_write(f, &v->items[i], indent, level + 1);
        }
        if (indent != 0) {
            fprintf(f, "\n%*s", indent * level, "");
        }
        fputc(v->type == PORT_JSON_ARRAY ? ']' : '}', f);
        break;
    }
}
