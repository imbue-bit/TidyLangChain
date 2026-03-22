#include "tidy_langchain/tlc_agent.h"
#include "tidy_langchain/tlc_string.h"
#include "tidy_langchain/tlc_json.h"
#include <string.h>

static tlc_status_t tlc_agent_build_prompt(tlc_agent_t* agent,
                                           const char* user_input,
                                           char* out,
                                           size_t out_size) {
    tlc_buffer_t buf;
    tlc_document_t docs[TLC_MAX_RETRIEVED_DOCS];
    size_t doc_count = 0;
    size_t i;

    if (!agent || !user_input || !out || out_size == 0) return TLC_ERR_INVALID_ARG;

    tlc_buf_init(&buf, out, out_size);

    if (tlc_buf_append(&buf,
        "You are TidyLangChain Agent.\n"
        "Rules:\n"
        "1. If a tool is needed, return a tool call.\n"
        "2. If no tool is needed, answer directly.\n"
        "3. Be concise and deterministic.\n") != TLC_OK) {
        return TLC_ERR_OVERFLOW;
    }

    if (agent->use_retrieval_context && agent->retriever) {
        if (agent->retriever->retrieve(agent->retriever, user_input, docs, TLC_MAX_RETRIEVED_DOCS, &doc_count) == TLC_OK) {
            if (doc_count > 0) {
                if (tlc_buf_append(&buf, "\nRetrieved context:\n") != TLC_OK) return TLC_ERR_OVERFLOW;
                for (i = 0; i < doc_count; ++i) {
                    if (tlc_buf_append(&buf, "- ") != TLC_OK) return TLC_ERR_OVERFLOW;
                    if (tlc_buf_append(&buf, docs[i].content) != TLC_OK) return TLC_ERR_OVERFLOW;
                    if (tlc_buf_append(&buf, "\n") != TLC_OK) return TLC_ERR_OVERFLOW;
                }
            }
        }
    }

    if (tlc_buf_append(&buf, "\nUser request:\n") != TLC_OK) return TLC_ERR_OVERFLOW;
    if (tlc_buf_append(&buf, user_input) != TLC_OK) return TLC_ERR_OVERFLOW;

    return TLC_OK;
}

void tlc_agent_init(tlc_agent_t* agent,
                    const char* name,
                    tlc_runtime_t* runtime,
                    tlc_callbacks_t* callbacks,
                    tlc_memory_t* memory,
                    tlc_model_t* model,
                    tlc_tool_registry_t* tools,
                    tlc_retriever_t* retriever) {
    if (!agent) return;
    memset(agent, 0, sizeof(*agent));
    if (name) tlc_strcpy_s(agent->name, sizeof(agent->name), name);
    agent->runtime = runtime;
    agent->callbacks = callbacks;
    agent->memory = memory;
    agent->model = model;
    agent->tools = tools;
    agent->retriever = retriever;
    agent->max_iterations = TLC_MAX_AGENT_ITERATIONS;
    agent->use_retrieval_context = true;
}

tlc_status_t tlc_agent_run(tlc_agent_t* agent,
                           const char* user_input,
                           char* output,
                           size_t output_size) {
    tlc_tool_def_t defs[TLC_MAX_TOOLS];
    size_t def_count = 0;
    char prompt[TLC_MAX_PROMPT_OUTPUT];
    char model_json[TLC_MAX_HTTP_RESPONSE_BODY];
    char tool_name[TLC_NAME_MAX];
    char tool_args[TLC_MAX_TOOL_ARGS];
    char tool_result[TLC_MAX_HTTP_RESPONSE_BODY];
    char final_answer[TLC_MAX_HTTP_RESPONSE_BODY];
    uint8_t iter;

    if (!agent || !user_input || !output || output_size == 0 || !agent->model) {
        return TLC_ERR_INVALID_ARG;
    }

    tlc_memory_add(agent->memory, TLC_ROLE_USER, NULL, user_input);

    def_count = tlc_tool_export_defs(agent->tools, defs, TLC_MAX_TOOLS);

    for (iter = 0; iter < agent->max_iterations; ++iter) {
        tlc_callback_log(agent->callbacks, "agent.iter.begin", agent->name);

        if (tlc_agent_build_prompt(agent, user_input, prompt, sizeof(prompt)) != TLC_OK) {
            return TLC_ERR_OVERFLOW;
        }

        if (agent->model->invoke(agent->model,
                                 agent->memory,
                                 prompt,
                                 defs,
                                 def_count,
                                 model_json,
                                 sizeof(model_json)) != TLC_OK) {
            return TLC_ERR_HTTP;
        }

        if (tlc_model_extract_first_tool_call(model_json,
                                              tool_name,
                                              sizeof(tool_name),
                                              tool_args,
                                              sizeof(tool_args)) == TLC_OK) {
            if (tlc_tool_call(agent->tools, tool_name, tool_args, tool_result, sizeof(tool_result)) != TLC_OK) {
                return TLC_ERR_TOOL;
            }

            tlc_memory_add(agent->memory, TLC_ROLE_TOOL, tool_name, tool_result);

            if (agent->model->invoke(agent->model,
                                     agent->memory,
                                     "Based on tool results, provide the final answer to the user.",
                                     defs,
                                     0,
                                     final_answer,
                                     sizeof(final_answer)) != TLC_OK) {
                return TLC_ERR_HTTP;
            }

            if (tlc_json_get_openai_message_content(final_answer, output, output_size) == TLC_OK) {
                tlc_memory_add(agent->memory, TLC_ROLE_ASSISTANT, NULL, output);
                return TLC_OK;
            }

            if (tlc_strcpy_s(output, output_size, final_answer) != TLC_OK) return TLC_ERR_OVERFLOW;
            tlc_memory_add(agent->memory, TLC_ROLE_ASSISTANT, NULL, output);
            return TLC_OK;
        } else {
            if (tlc_json_get_openai_message_content(model_json, output, output_size) == TLC_OK) {
                tlc_memory_add(agent->memory, TLC_ROLE_ASSISTANT, NULL, output);
                return TLC_OK;
            }
            if (tlc_strcpy_s(output, output_size, model_json) != TLC_OK) return TLC_ERR_OVERFLOW;
            tlc_memory_add(agent->memory, TLC_ROLE_ASSISTANT, NULL, output);
            return TLC_OK;
        }
    }

    return TLC_ERR_AGENT_STOP;
}
