#include "tidy_langchain/tlc_runtime.h"

const char* tlc_status_str(tlc_status_t st) {
    switch (st) {
        case TLC_OK: return "TLC_OK";
        case TLC_ERR_INVALID_ARG: return "TLC_ERR_INVALID_ARG";
        case TLC_ERR_OVERFLOW: return "TLC_ERR_OVERFLOW";
        case TLC_ERR_NOT_FOUND: return "TLC_ERR_NOT_FOUND";
        case TLC_ERR_UNSUPPORTED: return "TLC_ERR_UNSUPPORTED";
        case TLC_ERR_TRANSPORT: return "TLC_ERR_TRANSPORT";
        case TLC_ERR_HTTP: return "TLC_ERR_HTTP";
        case TLC_ERR_JSON: return "TLC_ERR_JSON";
        case TLC_ERR_PARSE: return "TLC_ERR_PARSE";
        case TLC_ERR_LIMIT: return "TLC_ERR_LIMIT";
        case TLC_ERR_INTERNAL: return "TLC_ERR_INTERNAL";
        case TLC_ERR_EMPTY: return "TLC_ERR_EMPTY";
        case TLC_ERR_TOOL: return "TLC_ERR_TOOL";
        case TLC_ERR_AGENT_STOP: return "TLC_ERR_AGENT_STOP";
        default: return "TLC_ERR_UNKNOWN";
    }
}

void tlc_runtime_init(tlc_runtime_t* rt, size_t memory_budget_bytes) {
    if (!rt) return;
    rt->memory_budget_bytes = memory_budget_bytes;
    rt->used_bytes = 0;
    rt->flags = 0;
}

tlc_status_t tlc_runtime_reserve(tlc_runtime_t* rt, size_t bytes) {
    if (!rt) return TLC_ERR_INVALID_ARG;
    if (rt->used_bytes + bytes > rt->memory_budget_bytes) {
        return TLC_ERR_LIMIT;
    }
    rt->used_bytes += bytes;
    return TLC_OK;
}

void tlc_runtime_release(tlc_runtime_t* rt, size_t bytes) {
    if (!rt) return;
    if (bytes > rt->used_bytes) {
        rt->used_bytes = 0;
        return;
    }
    rt->used_bytes -= bytes;
}
