#include "tidy_langchain/tlc_tool.h"
#include "tidy_langchain/tlc_string.h"
#include <string.h>

void tlc_tool_registry_init(tlc_tool_registry_t* reg) {
    if (!reg) return;
    memset(reg, 0, sizeof(*reg));
}

tlc_status_t tlc_tool_register(tlc_tool_registry_t* reg,
                               const char* name,
                               const char* description,
                               const char* json_schema,
                               tlc_tool_call_fn fn,
                               const tlc_tool_ctx_t* ctx) {
    tlc_tool_t* t;
    if (!reg || !name || !description || !fn) return TLC_ERR_INVALID_ARG;
    if (reg->count >= TLC_MAX_TOOLS) return TLC_ERR_LIMIT;

    t = &reg->tools[reg->count];
    memset(t, 0, sizeof(*t));
    if (tlc_strcpy_s(t->name, sizeof(t->name), name) != TLC_OK) return TLC_ERR_OVERFLOW;
    if (tlc_strcpy_s(t->description, sizeof(t->description), description) != TLC_OK) return TLC_ERR_OVERFLOW;
    if (json_schema && tlc_strcpy_s(t->json_schema, sizeof(t->json_schema), json_schema) != TLC_OK) {
        return TLC_ERR_OVERFLOW;
    }
    t->call = fn;
    if (ctx) t->ctx = *ctx;

    reg->count++;
    return TLC_OK;
}

const tlc_tool_t* tlc_tool_find(const tlc_tool_registry_t* reg, const char* name) {
    size_t i;
    if (!reg || !name) return NULL;
    for (i = 0; i < reg->count; ++i) {
        if (tlc_streq(reg->tools[i].name, name)) return &reg->tools[i];
    }
    return NULL;
}

tlc_status_t tlc_tool_call(const tlc_tool_registry_t* reg,
                           const char* name,
                           const char* args_json,
                           char* output,
                           size_t output_size) {
    const tlc_tool_t* t;
    if (!reg || !name || !output || output_size == 0) return TLC_ERR_INVALID_ARG;
    t = tlc_tool_find(reg, name);
    if (!t) return TLC_ERR_NOT_FOUND;
    return t->call((tlc_tool_ctx_t*)&t->ctx, args_json ? args_json : "{}", output, output_size);
}

size_t tlc_tool_export_defs(const tlc_tool_registry_t* reg,
                            tlc_tool_def_t* defs,
                            size_t max_defs) {
    size_t i, n;
    if (!reg || !defs || max_defs == 0) return 0;
    n = (reg->count < max_defs) ? reg->count : max_defs;
    for (i = 0; i < n; ++i) {
        memset(&defs[i], 0, sizeof(defs[i]));
        tlc_strcpy_s(defs[i].name, sizeof(defs[i].name), reg->tools[i].name);
        tlc_strcpy_s(defs[i].description, sizeof(defs[i].description), reg->tools[i].description);
        tlc_strcpy_s(defs[i].json_schema, sizeof(defs[i].json_schema), reg->tools[i].json_schema);
    }
    return n;
}
