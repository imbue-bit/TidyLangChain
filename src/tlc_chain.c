#include "tidy_langchain/tlc_chain.h"
#include "tidy_langchain/tlc_string.h"
#include "tidy_langchain/tlc_json.h"
#include <string.h>

void tlc_chain_init(tlc_chain_t* chain,
                    const char* name,
                    tlc_runtime_t* runtime,
                    tlc_callbacks_t* callbacks,
                    tlc_memory_t* memory,
                    tlc_tool_registry_t* tools) {
    if (!chain) return;
    memset(chain, 0, sizeof(*chain));
    if (name) tlc_strcpy_s(chain->name, sizeof(chain->name), name);
    chain->runtime = runtime;
    chain->callbacks = callbacks;
    chain->memory = memory;
    chain->tools = tools;
}

static tlc_status_t tlc_chain_add_step(tlc_chain_t* chain,
                                       tlc_chain_step_type_t type,
                                       const char* name,
                                       void* ref) {
    tlc_chain_step_t* step;
    if (!chain || !ref) return TLC_ERR_INVALID_ARG;
    if (chain->step_count >= TLC_MAX_CHAIN_STEPS) return TLC_ERR_LIMIT;

    step = &chain->steps[chain->step_count++];
    memset(step, 0, sizeof(*step));
    step->type = type;
    step->ref = ref;
    if (name) tlc_strcpy_s(step->name, sizeof(step->name), name);
    return TLC_OK;
}

tlc_status_t tlc_chain_add_prompt(tlc_chain_t* chain, tlc_prompt_template_t* pt) {
    return tlc_chain_add_step(chain, TLC_CHAIN_STEP_PROMPT, pt ? pt->name : "prompt", pt);
}

tlc_status_t tlc_chain_add_model(tlc_chain_t* chain, tlc_model_t* model) {
    return tlc_chain_add_step(chain, TLC_CHAIN_STEP_MODEL, model ? model->name : "model", model);
}

tlc_status_t tlc_chain_add_parser(tlc_chain_t* chain, tlc_output_parser_t* parser) {
    return tlc_chain_add_step(chain, TLC_CHAIN_STEP_PARSER, parser ? parser->name : "parser", parser);
}

tlc_status_t tlc_chain_add_tool(tlc_chain_t* chain, const char* tool_name) {
    return tlc_chain_add_step(chain, TLC_CHAIN_STEP_TOOL, tool_name, (void*)tool_name);
}

tlc_status_t tlc_chain_add_custom(tlc_chain_t* chain, tlc_runnable_t* runnable) {
    return tlc_chain_add_step(chain, TLC_CHAIN_STEP_CUSTOM, runnable ? runnable->name : "custom", runnable);
}

tlc_status_t tlc_chain_run(tlc_chain_t* chain,
                           const char* input,
                           char* output,
                           size_t output_size) {
    char a[TLC_MAX_PROMPT_OUTPUT];
    char b[TLC_MAX_HTTP_RESPONSE_BODY];
    char* cur_in = a;
    char* cur_out = b;
    size_t i;

    if (!chain || !input || !output || output_size == 0) return TLC_ERR_INVALID_ARG;
    if (tlc_strcpy_s(cur_in, TLC_MAX_PROMPT_OUTPUT, input) != TLC_OK) return TLC_ERR_OVERFLOW;

    for (i = 0; i < chain->step_count; ++i) {
        tlc_chain_step_t* step = &chain->steps[i];
        tlc_callback_log(chain->callbacks, "chain.step.begin", step->name);

        cur_out[0] = '\0';

        switch (step->type) {
            case TLC_CHAIN_STEP_PROMPT: {
                tlc_prompt_template_t* pt = (tlc_prompt_template_t*)step->ref;
                tlc_prompt_var_t vars[1];
                memset(vars, 0, sizeof(vars));
                tlc_strcpy_s(vars[0].key, sizeof(vars[0].key), "input");
                tlc_strcpy_s(vars[0].value, sizeof(vars[0].value), cur_in);
                if (tlc_prompt_format(pt, vars, 1, cur_out, TLC_MAX_HTTP_RESPONSE_BODY) != TLC_OK) {
                    return TLC_ERR_PARSE;
                }
                break;
            }
            case TLC_CHAIN_STEP_MODEL: {
                tlc_model_t* model = (tlc_model_t*)step->ref;
                if (model->invoke(model, chain->memory, cur_in, NULL, 0, cur_out, TLC_MAX_HTTP_RESPONSE_BODY) != TLC_OK) {
                    return TLC_ERR_HTTP;
                }
                break;
            }
            case TLC_CHAIN_STEP_PARSER: {
                tlc_output_parser_t* parser = (tlc_output_parser_t*)step->ref;
                if (parser->parse(parser, cur_in, cur_out, TLC_MAX_HTTP_RESPONSE_BODY) != TLC_OK) {
                    return TLC_ERR_PARSE;
                }
                break;
            }
            case TLC_CHAIN_STEP_TOOL: {
                const char* tool_name = (const char*)step->ref;
                if (!chain->tools) return TLC_ERR_TOOL;
                if (tlc_tool_call(chain->tools, tool_name, cur_in, cur_out, TLC_MAX_HTTP_RESPONSE_BODY) != TLC_OK) {
                    return TLC_ERR_TOOL;
                }
                break;
            }
            case TLC_CHAIN_STEP_CUSTOM: {
                tlc_runnable_t* runnable = (tlc_runnable_t*)step->ref;
                if (!runnable || !runnable->run) return TLC_ERR_INVALID_ARG;
                if (runnable->run(runnable, cur_in, cur_out, TLC_MAX_HTTP_RESPONSE_BODY) != TLC_OK) {
                    return TLC_ERR_INTERNAL;
                }
                break;
            }
            default:
                return TLC_ERR_UNSUPPORTED;
        }

        {
            char* tmp = cur_in;
            cur_in = cur_out;
            cur_out = tmp;
        }

        tlc_callback_log(chain->callbacks, "chain.step.end", step->name);
    }

    return tlc_strcpy_s(output, output_size, cur_in);
}
