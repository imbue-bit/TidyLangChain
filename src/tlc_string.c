#include "tidy_langchain/tlc_string.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

size_t tlc_strnlen_s(const char* s, size_t max_len) {
    size_t i;
    if (!s) return 0;
    for (i = 0; i < max_len; ++i) {
        if (s[i] == '\0') return i;
    }
    return max_len;
}

tlc_status_t tlc_strcpy_s(char* dst, size_t dst_size, const char* src) {
    size_t n;
    if (!dst || dst_size == 0 || !src) return TLC_ERR_INVALID_ARG;
    n = strlen(src);
    if (n + 1 > dst_size) return TLC_ERR_OVERFLOW;
    memcpy(dst, src, n + 1);
    return TLC_OK;
}

tlc_status_t tlc_strcat_s(char* dst, size_t dst_size, const char* src) {
    size_t dlen, slen;
    if (!dst || !src || dst_size == 0) return TLC_ERR_INVALID_ARG;
    dlen = strlen(dst);
    slen = strlen(src);
    if (dlen + slen + 1 > dst_size) return TLC_ERR_OVERFLOW;
    memcpy(dst + dlen, src, slen + 1);
    return TLC_OK;
}

tlc_status_t tlc_snprintf(char* dst, size_t dst_size, const char* fmt, ...) {
    int n;
    va_list ap;
    if (!dst || !fmt || dst_size == 0) return TLC_ERR_INVALID_ARG;
    va_start(ap, fmt);
    n = vsnprintf(dst, dst_size, fmt, ap);
    va_end(ap);
    if (n < 0) return TLC_ERR_INTERNAL;
    if ((size_t)n >= dst_size) return TLC_ERR_OVERFLOW;
    return TLC_OK;
}

bool tlc_streq(const char* a, const char* b) {
    if (!a || !b) return false;
    return strcmp(a, b) == 0;
}

bool tlc_str_contains(const char* s, const char* needle) {
    if (!s || !needle) return false;
    return strstr(s, needle) != NULL;
}

void tlc_trim(char* s) {
    char* end;
    char* start;
    size_t len;
    if (!s || !s[0]) return;

    start = s;
    while (*start && isspace((unsigned char)*start)) start++;

    if (start != s) {
        memmove(s, start, strlen(start) + 1);
    }

    len = strlen(s);
    if (len == 0) return;

    end = s + len - 1;
    while (end >= s && isspace((unsigned char)*end)) {
        *end = '\0';
        if (end == s) break;
        end--;
    }
}

void tlc_buf_init(tlc_buffer_t* buf, char* data, size_t size) {
    if (!buf || !data || size == 0) return;
    buf->data = data;
    buf->size = size;
    buf->len = 0;
    buf->data[0] = '\0';
}

tlc_status_t tlc_buf_append(tlc_buffer_t* buf, const char* s) {
    size_t n;
    if (!buf || !buf->data || !s) return TLC_ERR_INVALID_ARG;
    n = strlen(s);
    return tlc_buf_append_n(buf, s, n);
}

tlc_status_t tlc_buf_append_n(tlc_buffer_t* buf, const char* s, size_t n) {
    if (!buf || !buf->data || !s) return TLC_ERR_INVALID_ARG;
    if (buf->len + n + 1 > buf->size) return TLC_ERR_OVERFLOW;
    memcpy(buf->data + buf->len, s, n);
    buf->len += n;
    buf->data[buf->len] = '\0';
    return TLC_OK;
}

tlc_status_t tlc_buf_append_char(tlc_buffer_t* buf, char c) {
    if (!buf || !buf->data) return TLC_ERR_INVALID_ARG;
    if (buf->len + 2 > buf->size) return TLC_ERR_OVERFLOW;
    buf->data[buf->len++] = c;
    buf->data[buf->len] = '\0';
    return TLC_OK;
}

void tlc_buf_clear(tlc_buffer_t* buf) {
    if (!buf || !buf->data || buf->size == 0) return;
    buf->len = 0;
    buf->data[0] = '\0';
}

tlc_status_t tlc_json_escape_into(const char* src, char* dst, size_t dst_size) {
    tlc_buffer_t buf;
    size_t i;
    if (!src || !dst || dst_size == 0) return TLC_ERR_INVALID_ARG;

    tlc_buf_init(&buf, dst, dst_size);

    for (i = 0; src[i] != '\0'; ++i) {
        char c = src[i];
        switch (c) {
            case '\"':
                if (tlc_buf_append(&buf, "\\\"") != TLC_OK) return TLC_ERR_OVERFLOW;
                break;
            case '\\':
                if (tlc_buf_append(&buf, "\\\\") != TLC_OK) return TLC_ERR_OVERFLOW;
                break;
            case '\n':
                if (tlc_buf_append(&buf, "\\n") != TLC_OK) return TLC_ERR_OVERFLOW;
                break;
            case '\r':
                if (tlc_buf_append(&buf, "\\r") != TLC_OK) return TLC_ERR_OVERFLOW;
                break;
            case '\t':
                if (tlc_buf_append(&buf, "\\t") != TLC_OK) return TLC_ERR_OVERFLOW;
                break;
            default:
                if ((unsigned char)c < 0x20) {
                    return TLC_ERR_UNSUPPORTED;
                }
                if (tlc_buf_append_char(&buf, c) != TLC_OK) return TLC_ERR_OVERFLOW;
                break;
        }
    }
    return TLC_OK;
}
