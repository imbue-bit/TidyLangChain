#ifndef TLC_TOOL_H
#define TLC_TOOL_H

#include "tlc_model.h"
#include "tlc_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlc_tool_ctx_s {
    tlc_runtime_t* runtime;
    tlc_callbacks_t* callbacks;
    void* user_data;
} tlc_tool_ctx_t;

typedef tlc_status_t (*tlc_tool_call_fn)(tlc_tool_ctx_t* ctx,
                                         const char* args_json,
                                         char* output,
                                         size_t output_size);

typedef struct tlc_tool_s {
    char name[TLC_NAME_MAX];
    char description[TLC_MAX_TOOL_DESC];
    char json_schema[TLC_MAX_TOOL_SCHEMA];
    tlc_tool_call_fn call;
    tlc_tool_ctx_t ctx;
} tlc_tool_t;

typedef struct tlc_tool_registry_s {
    tlc_tool_t tools[TLC_MAX_TOOLS];
    size_t count;
} tlc_tool_registry_t;

void tlc_tool_registry_init(tlc_tool_registry_t* reg);

tlc_status_t tlc_tool_register(tlc_tool_registry_t* reg,
                               const char* name,
                               const char* description,
                               const char* json_schema,
                               tlc_tool_call_fn fn,
                               const tlc_tool_ctx_t* ctx);

const tlc_tool_t* tlc_tool_find(const tlc_tool_registry_t* reg, const char* name);

tlc_status_t tlc_tool_call(const tlc_tool_registry_t* reg,
                           const char* name,
                           const char* args_json,
                           char* output,
                           size_t output_size);

size_t tlc_tool_export_defs(const tlc_tool_registry_t* reg,
                            tlc_tool_def_t* defs,
                            size_t max_defs);

#ifdef __cplusplus
}
#endif

#endif
