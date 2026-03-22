#ifndef TLC_CONFIG_H
#define TLC_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* 通用短字符串 */
#ifndef TLC_NAME_MAX
#define TLC_NAME_MAX                 32
#endif

#ifndef TLC_SMALL_STR
#define TLC_SMALL_STR                64
#endif

#ifndef TLC_MEDIUM_STR
#define TLC_MEDIUM_STR               128
#endif

#ifndef TLC_LARGE_STR
#define TLC_LARGE_STR                256
#endif

/* 消息 / 文档 / 工具容量 */
#ifndef TLC_MAX_MESSAGES
#define TLC_MAX_MESSAGES             32
#endif

#ifndef TLC_MAX_MESSAGE_CONTENT
#define TLC_MAX_MESSAGE_CONTENT      1024
#endif

#ifndef TLC_MAX_PROMPT_TEMPLATE
#define TLC_MAX_PROMPT_TEMPLATE      1024
#endif

#ifndef TLC_MAX_PROMPT_OUTPUT
#define TLC_MAX_PROMPT_OUTPUT        2048
#endif

#ifndef TLC_MAX_HTTP_HEADERS
#define TLC_MAX_HTTP_HEADERS         16
#endif

#ifndef TLC_MAX_HTTP_HEADER_KEY
#define TLC_MAX_HTTP_HEADER_KEY      64
#endif

#ifndef TLC_MAX_HTTP_HEADER_VALUE
#define TLC_MAX_HTTP_HEADER_VALUE    256
#endif

#ifndef TLC_MAX_HTTP_REQUEST_BODY
#define TLC_MAX_HTTP_REQUEST_BODY    8192
#endif

#ifndef TLC_MAX_HTTP_RESPONSE_BODY
#define TLC_MAX_HTTP_RESPONSE_BODY   16384
#endif

#ifndef TLC_MAX_TOOLS
#define TLC_MAX_TOOLS                16
#endif

#ifndef TLC_MAX_TOOL_DESC
#define TLC_MAX_TOOL_DESC            256
#endif

#ifndef TLC_MAX_TOOL_SCHEMA
#define TLC_MAX_TOOL_SCHEMA          1024
#endif

#ifndef TLC_MAX_CHAIN_STEPS
#define TLC_MAX_CHAIN_STEPS          16
#endif

#ifndef TLC_MAX_DOCUMENTS
#define TLC_MAX_DOCUMENTS            32
#endif

#ifndef TLC_MAX_DOCUMENT_CONTENT
#define TLC_MAX_DOCUMENT_CONTENT     1024
#endif

#ifndef TLC_MAX_METADATA
#define TLC_MAX_METADATA             256
#endif

#ifndef TLC_MAX_VECTOR_DIM
#define TLC_MAX_VECTOR_DIM           256
#endif

#ifndef TLC_MAX_VECTOR_ITEMS
#define TLC_MAX_VECTOR_ITEMS         32
#endif

#ifndef TLC_MAX_RETRIEVED_DOCS
#define TLC_MAX_RETRIEVED_DOCS       8
#endif

#ifndef TLC_MAX_AGENT_SCRATCHPAD
#define TLC_MAX_AGENT_SCRATCHPAD     4096
#endif

#ifndef TLC_MAX_AGENT_ITERATIONS
#define TLC_MAX_AGENT_ITERATIONS     6
#endif

#ifndef TLC_MAX_TOOL_ARGS
#define TLC_MAX_TOOL_ARGS            2048
#endif

#ifndef TLC_ENABLE_STDIO_LOG
#define TLC_ENABLE_STDIO_LOG         1
#endif

/* 是否启用 JSON 构造/解析 */
#ifndef TLC_ENABLE_MINI_JSON
#define TLC_ENABLE_MINI_JSON         1
#endif

/* OpenAI chat 接口默认路径 */
#ifndef TLC_DEFAULT_OPENAI_CHAT_PATH
#define TLC_DEFAULT_OPENAI_CHAT_PATH "/v1/chat/completions"
#endif

#ifdef __cplusplus
}
#endif

#endif
