#ifndef TLC_JSON_H
#define TLC_JSON_H

#include "tlc_types.h"
#include "tlc_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 从 JSON 文本中查找第一个字符串字段 key 的值 */
tlc_status_t tlc_json_get_string(const char* json,
                                 const char* key,
                                 char* out,
                                 size_t out_size);

/* 从 JSON 文本中提取 choices[0].message.content */
tlc_status_t tlc_json_get_openai_message_content(const char* json,
                                                 char* out,
                                                 size_t out_size);

/* 从 JSON 文本中提取 tool_calls[0].function.name */
tlc_status_t tlc_json_get_openai_first_tool_name(const char* json,
                                                 char* out,
                                                 size_t out_size);

/* 从 JSON 文本中提取 tool_calls[0].function.arguments */
tlc_status_t tlc_json_get_openai_first_tool_args(const char* json,
                                                 char* out,
                                                 size_t out_size);

#ifdef __cplusplus
}
#endif

#endif
