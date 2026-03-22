#ifndef TLC_MODEL_H
#define TLC_MODEL_H

#include "tlc_memory.h"
#include "tlc_transport.h"
#include "tlc_callbacks.h"
#include "tlc_runtime.h"
#include "tlc_json.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlc_chat_options_s {
    char model[TLC_SMALL_STR];
    float temperature;
    uint16_t max_tokens;
    bool enable_tools;
} tlc_chat_options_t;

typedef struct tlc_tool_def_s {
    char name[TLC_NAME_MAX];
    char description[TLC_MAX_TOOL_DESC];
    char json_schema[TLC_MAX_TOOL_SCHEMA];
} tlc_tool_def_t;

typedef struct tlc_model_s tlc_model_t;

typedef tlc_status_t (*tlc_model_invoke_fn)(tlc_model_t* model,
                                            const tlc_memory_t* memory,
                                            const char* prompt,
                                            const tlc_tool_def_t* tools,
                                            size_t tool_count,
                                            char* out,
                                            size_t out_size);

struct tlc_model_s {
    char name[TLC_NAME_MAX];
    tlc_runtime_t* runtime;
    tlc_callbacks_t* callbacks;
    tlc_transport_t transport;

    char api_key[TLC_LARGE_STR];
    char host[TLC_MEDIUM_STR];
    uint16_t port;
    bool use_tls;
    char path[TLC_LARGE_STR];

    tlc_chat_options_t options;

    tlc_model_invoke_fn invoke;
};

void tlc_chat_options_default(tlc_chat_options_t* opt);

tlc_status_t tlc_model_openai_init(tlc_model_t* model,
                                   tlc_runtime_t* runtime,
                                   tlc_callbacks_t* callbacks,
                                   const tlc_transport_t* transport,
                                   const char* host,
                                   uint16_t port,
                                   bool use_tls,
                                   const char* path,
                                   const char* api_key,
                                   const tlc_chat_options_t* options);

tlc_status_t tlc_model_openai_invoke(tlc_model_t* model,
                                     const tlc_memory_t* memory,
                                     const char* prompt,
                                     const tlc_tool_def_t* tools,
                                     size_t tool_count,
                                     char* out,
                                     size_t out_size);

tlc_status_t tlc_model_extract_first_tool_call(const char* model_json,
                                               char* tool_name,
                                               size_t tool_name_size,
                                               char* tool_args_json,
                                               size_t tool_args_size);

#ifdef __cplusplus
}
#endif

#endif
