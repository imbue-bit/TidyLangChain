#ifndef TLC_STRING_H
#define TLC_STRING_H

#include "tlc_types.h"
#include "tlc_status.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t tlc_strnlen_s(const char* s, size_t max_len);
tlc_status_t tlc_strcpy_s(char* dst, size_t dst_size, const char* src);
tlc_status_t tlc_strcat_s(char* dst, size_t dst_size, const char* src);
tlc_status_t tlc_snprintf(char* dst, size_t dst_size, const char* fmt, ...);
bool tlc_streq(const char* a, const char* b);
bool tlc_str_contains(const char* s, const char* needle);
void tlc_trim(char* s);
void tlc_buf_init(tlc_buffer_t* buf, char* data, size_t size);
tlc_status_t tlc_buf_append(tlc_buffer_t* buf, const char* s);
tlc_status_t tlc_buf_append_n(tlc_buffer_t* buf, const char* s, size_t n);
tlc_status_t tlc_buf_append_char(tlc_buffer_t* buf, char c);
void tlc_buf_clear(tlc_buffer_t* buf);

tlc_status_t tlc_json_escape_into(const char* src, char* dst, size_t dst_size);

#ifdef __cplusplus
}
#endif

#endif
