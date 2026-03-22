#ifndef TLC_CHAIN_H
#define TLC_CHAIN_H

#include "tlc_prompt.h"
#include "tlc_parser.h"
#include "tlc_tool.h"
#include "tlc_runnable.h"
#include "tlc_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlc_chain_step_s {
    tlc_chain_step_type_t type;
    char name[TLC_NAME_MAX];
    void* ref;
} tlc_chain_step_t;

typedef struct tlc_chain_s {
    char name[TLC_NAME_MAX];
    tlc_runtime_t* runtime;
    tlc_callbacks_t* callbacks;
    tlc_memory_t* memory;
    tlc_tool_registry_t* tools;
    tlc_chain_step_t steps[TLC_MAX_CHAIN_STEPS];
    size_t step_count;
} tlc_chain_t;

void tlc_chain_init(tlc_chain_t* chain,
                    const char* name,
                    tlc_runtime_t* runtime,
                    tlc_callbacks_t* callbacks,
                    tlc_memory_t* memory,
                    tlc_tool_registry_t* tools);

tlc_status_t tlc_chain_add_prompt(tlc_chain_t* chain, tlc_prompt_template_t* pt);
tlc_status_t tlc_chain_add_model(tlc_chain_t* chain, tlc_model_t* model);
tlc_status_t tlc_chain_add_parser(tlc_chain_t* chain, tlc_output_parser_t* parser);
tlc_status_t tlc_chain_add_tool(tlc_chain_t* chain, const char* tool_name);
tlc_status_t tlc_chain_add_custom(tlc_chain_t* chain, tlc_runnable_t* runnable);

tlc_status_t tlc_chain_run(tlc_chain_t* chain,
                           const char* input,
                           char* output,
                           size_t output_size);

#ifdef __cplusplus
}
#endif

#endif
