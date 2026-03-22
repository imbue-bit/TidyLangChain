#ifndef TLC_RUNTIME_H
#define TLC_RUNTIME_H

#include "tlc_types.h"
#include "tlc_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlc_runtime_s {
    size_t memory_budget_bytes;
    size_t used_bytes;
    uint32_t flags;
} tlc_runtime_t;

void tlc_runtime_init(tlc_runtime_t* rt, size_t memory_budget_bytes);
tlc_status_t tlc_runtime_reserve(tlc_runtime_t* rt, size_t bytes);
void tlc_runtime_release(tlc_runtime_t* rt, size_t bytes);

#ifdef __cplusplus
}
#endif

#endif
