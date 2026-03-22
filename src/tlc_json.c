#include "tidy_langchain/tlc_json.h"
#include "tidy_langchain/tlc_string.h"
#include <string.h>
#include <ctype.h>

static const char* tlc_skip_ws(const char* p) {
    while (p && *p && isspace((unsigned char)*p)) p++;
    return p;
}

static const char* tlc_find_key(const char* json, const char* key) {
    char pattern[TLC_SMALL_STR + 4];
    if (!json || !key) return NULL;
    if (tlc_snprintf(pattern, sizeof(pattern), "\"%s\"", key) != TLC_OK) return NULL;
    return strstr(json, pattern);
}

static tlc_status_t tlc_extract_json_string_value(const char* p, char* out, size_t out_size) {
    size_t j = 0;
    bool esc = false;

    if (!p || !out || out_size == 0) return TLC_ERR_INVALID_ARG;
    p = strchr(p, ':');
    if (!p) return TLC_ERR_JSON;
    p++;
    p = tlc_skip_ws(p);
    if (*p != '"') return TLC_ERR_JSON;
    p++;

    while (*p) {
        char c = *p++;
        if (esc) {
            switch (c) {
                case '"': c = '"'; break;
                case '\\': c = '\\'; break;
                case 'n': c = '\n'; break;
                case 'r': c = '\r'; break;
                case 't': c = '\t'; break;
                default: break;
            }
            if (j + 1 >= out_size) return TLC_ERR_OVERFLOW;
            out[j++] = c;
            esc = false;
            continue;
        }
        if (c == '\\') {
            esc = true;
            continue;
        }
        if (c == '"') {
            if (j >= out_size) return TLC_ERR_OVERFLOW;
            out[j] = '\0';
            return TLC_OK;
        }
        if (j + 1 >= out_size) return TLC_ERR_OVERFLOW;
        out[j++] = c;
    }
    return TLC_ERR_JSON;
}

static tlc_status_t tlc_extract_json_object_string_field(const char* start,
                                                         const char* field,
                                                         char* out,
                                                         size_t out_size) {
    const char* p = tlc_find_key(start, field);
    if (!p) return TLC_ERR_NOT_FOUND;
    return tlc_extract_json_string_value(p, out, out_size);
}

tlc_status_t tlc_json_get_string(const char* json,
                                 const char* key,
                                 char* out,
                                 size_t out_size) {
    const char* p;
    if (!json || !key || !out || out_size == 0) return TLC_ERR_INVALID_ARG;
    p = tlc_find_key(json, key);
    if (!p) return TLC_ERR_NOT_FOUND;
    return tlc_extract_json_string_value(p, out, out_size);
}

tlc_status_t tlc_json_get_openai_message_content(const char* json,
                                                 char* out,
                                                 size_t out_size) {
    const char* choices;
    const char* message;
    if (!json || !out || out_size == 0) return TLC_ERR_INVALID_ARG;

    choices = strstr(json, "\"choices\"");
    if (!choices) return TLC_ERR_NOT_FOUND;

    message = strstr(choices, "\"message\"");
    if (!message) return TLC_ERR_NOT_FOUND;

    return tlc_extract_json_object_string_field(message, "content", out, out_size);
}

tlc_status_t tlc_json_get_openai_first_tool_name(const char* json,
                                                 char* out,
                                                 size_t out_size) {
    const char* tc;
    const char* func;
    if (!json || !out || out_size == 0) return TLC_ERR_INVALID_ARG;

    tc = strstr(json, "\"tool_calls\"");
    if (!tc) return TLC_ERR_NOT_FOUND;

    func = strstr(tc, "\"function\"");
    if (!func) return TLC_ERR_NOT_FOUND;

    return tlc_extract_json_object_string_field(func, "name", out, out_size);
}

tlc_status_t tlc_json_get_openai_first_tool_args(const char* json,
                                                 char* out,
                                                 size_t out_size) {
    const char* tc;
    const char* func;
    if (!json || !out || out_size == 0) return TLC_ERR_INVALID_ARG;

    tc = strstr(json, "\"tool_calls\"");
    if (!tc) return TLC_ERR_NOT_FOUND;

    func = strstr(tc, "\"function\"");
    if (!func) return TLC_ERR_NOT_FOUND;

    return tlc_extract_json_object_string_field(func, "arguments", out, out_size);
}
