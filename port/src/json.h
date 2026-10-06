/* A small, strict JSON reader (RFC 8259) for the input scripts (script.c) and the settings (settings.c): objects,
 * arrays, strings with every escape (\uXXXX and surrogate pairs become UTF-8), numbers, true, false, null; and a
 * writer. Our own code; it needs only libc.
 *
 * Strict: one value and nothing after it but whitespace, no trailing commas, no comments, no duplicate keys in an
 * object, no control characters in strings, numbers in JSON's grammar only (no leading zeros, no hex, no NaN), at
 * most PORT_JSON_MAX_DEPTH nested arrays/objects. Numbers are kept as doubles (exact for integers up to 2^53). */
#ifndef PORT_JSON_H
#define PORT_JSON_H

#include <stddef.h>
#include <stdio.h>

#define PORT_JSON_MAX_DEPTH 64

typedef enum PortJsonType {
    PORT_JSON_NULL,
    PORT_JSON_BOOL,
    PORT_JSON_NUMBER,
    PORT_JSON_STRING,
    PORT_JSON_ARRAY,
    PORT_JSON_OBJECT,
} PortJsonType;

typedef struct PortJson {
    PortJsonType type;
    int boolean;            /* PORT_JSON_BOOL */
    double number;          /* PORT_JSON_NUMBER */
    char *string;           /* PORT_JSON_STRING: NUL-terminated UTF-8 (an embedded \u0000 ends it early) */
    size_t count;           /* PORT_JSON_ARRAY / PORT_JSON_OBJECT: the number of items */
    struct PortJson *items; /* the items, in their order in the text */
    char **keys;            /* PORT_JSON_OBJECT: keys[i] names items[i] */
} PortJson;

/* Parses `text` (`length` bytes). Returns the root value, or NULL with a message ("line L column C: ...") in `err`. */
PortJson *port_json_parse(const char *text, size_t length, char *err, size_t err_size);
/* The member `key` of an object, or NULL (also when `obj` is NULL or not an object). */
const PortJson *port_json_get(const PortJson *obj, const char *key);
/* The type's name ("null", "boolean", "number", "string", "array", "object"), for messages. */
const char *port_json_type_name(PortJsonType type);
void port_json_free(PortJson *value);
/* The writer: `s` as a JSON string; `v` as JSON text, `indent` spaces a nesting level, starting at `level`. */
void port_json_write_string(FILE *f, const char *s);
void port_json_write(FILE *f, const PortJson *v, int indent, int level);

#endif /* PORT_JSON_H */
