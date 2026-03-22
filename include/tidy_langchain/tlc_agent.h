#ifndef TLC_AGENT_H
#define TLC_AGENT_H

#include "tlc_chain.h"
#include "tlc_retriever.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlc_agent_s {
    char name[TLC_NAME_MAX];
    tlc_runtime_t* runtime;
    tlc_callbacks_t* callbacks;
    tlc_memory_t* memory;
    tlc_model_t* model;
    tlc_tool_registry_t* tools;
    tlc_retriever_t* retriever;
    uint8_t max_iterations;
    bool use_retrieval_context;
} tlc_agent_t;

void tlc_agent_init(tlc_agent_t* agent,
                    const char* name,
                    tlc_runtime_t* runtime,
                    tlc_callbacks_t* callbacks,
                    tlc_memory_t* memory,
                    tlc_model_t* model,
                    tlc_tool_registry_t* tools,
                    tlc_retriever_t* retriever);

tlc_status_t tlc_agent_run(tlc_agent_t* agent,
                           const char* user_input,
                           char* output,
                           size_t output_size);

#ifdef __cplusplus
}
#endif

#endif
