#include "tidy_langchain/tidy_langchain.h"
#include <stdio.h>
#include <string.h>

/* ---------- 简单日志 ---------- */
static void example_log(void* user, const char* event, const char* detail) {
    (void)user;
    printf("[LOG] %s: %s\n", event, detail ? detail : "");
}

/* ---------- Mock HTTP Transport ---------- */
static tlc_status_t mock_send(void* user_ctx,
                              const tlc_http_request_t* req,
                              tlc_http_response_t* resp) {
    (void)user_ctx;
    printf("=== HTTP REQUEST ===\n");
    printf("%s https://%s:%u%s\n", req->method, req->host, req->port, req->path);
    printf("BODY: %s\n", req->body);

    resp->status_code = 200;

    /* 如果 prompt 中含有 weather，就模拟 tool call */
    if (strstr(req->body, "weather") || strstr(req->body, "天气")) {
        tlc_strcpy_s(resp->body, sizeof(resp->body),
            "{"
            "\"id\":\"chatcmpl_mock\","
            "\"choices\":[{"
                "\"message\":{"
                    "\"role\":\"assistant\","
                    "\"content\":\"\","
                    "\"tool_calls\":[{"
                        "\"id\":\"call_1\","
                        "\"type\":\"function\","
                        "\"function\":{"
                            "\"name\":\"get_weather\","
                            "\"arguments\":\"{\\\"city\\\":\\\"Shanghai\\\"}\""
                        "}"
                    "}]"
                "}"
            "}]"
            "}");
    } else if (strstr(req->body, "Based on tool results")) {
        tlc_strcpy_s(resp->body, sizeof(resp->body),
            "{"
            "\"id\":\"chatcmpl_mock\","
            "\"choices\":[{"
                "\"message\":{"
                    "\"role\":\"assistant\","
                    "\"content\":\"根据工具结果，上海当前天气晴，气温 26C。\""
                "}"
            "}]"
            "}");
    } else {
        tlc_strcpy_s(resp->body, sizeof(resp->body),
            "{"
            "\"id\":\"chatcmpl_mock\","
            "\"choices\":[{"
                "\"message\":{"
                    "\"role\":\"assistant\","
                    "\"content\":\"这是一个直接回答，不需要调用工具。\""
                "}"
            "}]"
            "}");
    }

    return TLC_OK;
}

/* ---------- 示例 Tool: get_weather ---------- */
static tlc_status_t get_weather_tool(tlc_tool_ctx_t* ctx,
                                     const char* args_json,
                                     char* output,
                                     size_t output_size) {
    (void)ctx;
    (void)args_json;
    return tlc_strcpy_s(output, output_size,
                        "{\"city\":\"Shanghai\",\"condition\":\"Sunny\",\"temp_c\":26}");
}

int main(void) {
    tlc_runtime_t runtime;
    tlc_callbacks_t callbacks;
    tlc_transport_t transport;
    tlc_chat_options_t options;
    tlc_model_t model;
    tlc_memory_t memory;
    tlc_tool_registry_t tools;
    tlc_agent_t agent;
    tlc_tool_ctx_t tool_ctx;
    char result[2048];

    tlc_runtime_init(&runtime, 1024 * 1024);

    callbacks.on_log = example_log;
    callbacks.user = NULL;

    transport.send = mock_send;
    transport.user_ctx = NULL;

    tlc_chat_options_default(&options);
    tlc_strcpy_s(options.model, sizeof(options.model), "gpt-4o-mini");
    options.enable_tools = true;

    tlc_model_openai_init(&model,
                          &runtime,
                          &callbacks,
                          &transport,
                          "api.openai.com",
                          443,
                          true,
                          "/v1/chat/completions",
                          "sk-demo",
                          &options);

    tlc_memory_init(&memory);
    tlc_tool_registry_init(&tools);

    tool_ctx.runtime = &runtime;
    tool_ctx.callbacks = &callbacks;
    tool_ctx.user_data = NULL;

    tlc_tool_register(&tools,
                      "get_weather",
                      "Get current weather by city name",
                      "{\"type\":\"object\",\"properties\":{\"city\":{\"type\":\"string\"}},\"required\":[\"city\"]}",
                      get_weather_tool,
                      &tool_ctx);

    tlc_agent_init(&agent,
                   "demo_agent",
                   &runtime,
                   &callbacks,
                   &memory,
                   &model,
                   &tools,
                   NULL);

    /* 1) 直接回答 */
    if (tlc_agent_run(&agent, "请简单介绍一下你自己", result, sizeof(result)) == TLC_OK) {
        printf("\n[AGENT RESULT 1]\n%s\n", result);
    }

    /* 2) 工具调用 */
    if (tlc_agent_run(&agent, "帮我查一下 weather", result, sizeof(result)) == TLC_OK) {
        printf("\n[AGENT RESULT 2]\n%s\n", result);
    }

    /* 3) Chain 示例 */
    {
        tlc_prompt_template_t pt;
        tlc_chain_t chain;
        tlc_output_parser_t parser;
        char chain_out[2048];

        tlc_prompt_template_init(&pt, "simple_prompt", "请重写下面内容:\n{input}");
        memset(&parser, 0, sizeof(parser));
        tlc_strcpy_s(parser.name, sizeof(parser.name), "text_parser");
        parser.parse = tlc_text_output_parser;

        tlc_chain_init(&chain, "rewrite_chain", &runtime, &callbacks, &memory, &tools);
        tlc_chain_add_prompt(&chain, &pt);
        tlc_chain_add_model(&chain, &model);
        tlc_chain_add_parser(&chain, &parser);

        if (tlc_chain_run(&chain, "今天心情很好", chain_out, sizeof(chain_out)) == TLC_OK) {
            printf("\n[CHAIN RESULT]\n%s\n", chain_out);
        }
    }

    return 0;
}
