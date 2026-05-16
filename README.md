<div align="center">

# ⚛️ TidyLangChain

English | [简体中文](README_CN.md)

**A Deterministic Framework for LLM Agent Orchestration in Resource-Constrained Environments**

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Standard](https://img.shields.io/badge/Standard-ANSI%20C11-00599C.svg)]()
[![Platform](https://img.shields.io/badge/Platform-MCU%20%7C%20POSIX-lightgrey.svg)]()

[**Abstract**](#-abstract) •
[**Architecture**](#-system-architecture) •
[**Implementation Paradigm**](#-implementation-paradigm) •
[**Build Instructions**](#-build-instructions)

</div>

<br/>

## 📋 Abstract

**TidyLangChain** is an embedded-centric software framework designed to facilitate the orchestration of Large Language Models (LLMs) and autonomous agents on Microcontroller Units (MCUs) and edge devices. Implemented strictly in ANSI C11, the framework translates the cognitive orchestration principles of LangChain into a deterministic, memory-bounded execution environment. 

By utilizing pre-allocated memory budgets and a decoupled Hardware Abstraction Layer (HAL) for network transport, TidyLangChain guarantees temporal and spatial determinism, making it suitable for RTOS-based IoT devices requiring on-device intelligence, sensor-actuator tool invocation, and multi-step reasoning.

## 🏛️ System Architecture

The framework is structurally partitioned into multiple decoupled modules, ensuring minimal footprint and high portability:

*   **Runtime & Memory (`tlc_runtime`)**: Employs continuous memory pooling (`tlc_runtime_init`) to prevent heap fragmentation. All allocations are tracked within a strict predefined memory budget.
*   **Hardware Abstraction Layer (`tlc_transport`)**: Network operations are completely abstracted via function pointers, allowing agnostic binding to LwIP, AT command sequences, or POSIX sockets.
*   **Cognitive Orchestration (`tlc_agent`, `tlc_chain`)**: Implements sequential processing pipelines (Chains) and reactive state machines (Agents) that dynamically map LLM outputs to local function executions.
*   **External Tool Registry (`tlc_tool`)**: Provides a standardized interface for defining JSON-schema-based hardware tools (e.g., reading sensors, actuating GPIOs) executable by the LLM.

## 💻 Implementation Paradigm

The following code illustrates the complete lifecycle of integrating a deterministic Agent and a linear Chain, utilizing a mock HTTP transport layer to simulate network operations.

```c
#include "tidy_langchain/tidy_langchain.h"
#include <stdio.h>
#include <string.h>

/* ---------- 1. Callback & Logging Subsystem ---------- */
static void example_log(void* user, const char* event, const char* detail) {
    (void)user;
    printf("[LOG] %s: %s\n", event, detail ? detail : "");
}

/* ---------- 2. Transport Abstraction Layer ---------- */
static tlc_status_t mock_send(void* user_ctx,
                              const tlc_http_request_t* req,
                              tlc_http_response_t* resp) {
    (void)user_ctx;
    printf("=== HTTP REQUEST ===\n");
    printf("%s https://%s:%u%s\n", req->method, req->host, req->port, req->path);
    printf("BODY: %s\n", req->body);

    resp->status_code = 200;

    /* Simulated response generation based on payload content */
    if (strstr(req->body, "Based on tool results")) {
        tlc_strcpy_s(resp->body, sizeof(resp->body),
            "{\"id\":\"chatcmpl_mock\",\"choices\":[{\"message\":{"
            "\"role\":\"assistant\",\"content\":\"根据工具结果，上海当前天气晴，气温 26C。\"}}]}");
    } else {
        /* Keyword detection for tool triggering */
        const char *last_user = NULL;
        const char *p = req->body;
        bool is_weather = false;
        while ((p = strstr(p, "\"role\":\"user\"")) != NULL) {
            last_user = p;
            p += 13;
        }
        if (last_user) {
            const char *cnt = strstr(last_user, "\"content\":\"");
            if (cnt) {
                cnt += 11;
                const char *end = strchr(cnt, '"');
                if (end) {
                    size_t len = (size_t)(end - cnt);
                    char snippet[512];
                    if (len < sizeof(snippet)) {
                        memcpy(snippet, cnt, len);
                        snippet[len] = '\0';
                        if (strstr(snippet, "weather") || strstr(snippet, "天气")) {
                            is_weather = true;
                        }
                    }
                }
            }
        }
        
        if (is_weather) {
            /* Emit Tool Call Payload */
            tlc_strcpy_s(resp->body, sizeof(resp->body),
                "{\"id\":\"chatcmpl_mock\",\"choices\":[{\"message\":{"
                "\"role\":\"assistant\",\"content\":\"\",\"tool_calls\":[{"
                "\"id\":\"call_1\",\"type\":\"function\",\"function\":{"
                "\"name\":\"get_weather\",\"arguments\":\"{\\\"city\\\":\\\"Shanghai\\\"}\"}}]}}]}");
        } else {
            /* Emit Direct Response Payload */
            tlc_strcpy_s(resp->body, sizeof(resp->body),
                "{\"id\":\"chatcmpl_mock\",\"choices\":[{\"message\":{"
                "\"role\":\"assistant\",\"content\":\"这是一个直接回答，不需要调用工具。\"}}]}");
        }
    }
    return TLC_OK;
}

/* ---------- 3. Hardware Tool Definition ---------- */
static tlc_status_t get_weather_tool(tlc_tool_ctx_t* ctx,
                                     const char* args_json,
                                     char* output,
                                     size_t output_size) {
    (void)ctx;
    (void)args_json;
    /* In a real implementation, this would read an I2C/SPI sensor */
    return tlc_strcpy_s(output, output_size,
                        "{\"city\":\"Shanghai\",\"condition\":\"Sunny\",\"temp_c\":26}");
}

/* ---------- 4. Main Execution & Orchestration ---------- */
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

    /* Initialize runtime with a deterministic 1MB memory budget */
    tlc_runtime_init(&runtime, 1024 * 1024);

    callbacks.on_log = example_log;
    callbacks.user = NULL;

    transport.send = mock_send;
    transport.user_ctx = NULL;

    tlc_chat_options_default(&options);
    tlc_strcpy_s(options.model, sizeof(options.model), "gpt-4o-mini");
    options.enable_tools = true;

    /* Initialize Model I/O Context */
    tlc_model_openai_init(&model, &runtime, &callbacks, &transport,
                          "api.openai.com", 443, true,
                          "/v1/chat/completions", "sk-demo", &options);

    tlc_memory_init(&memory);
    tlc_tool_registry_init(&tools);

    tool_ctx.runtime = &runtime;
    tool_ctx.callbacks = &callbacks;
    tool_ctx.user_data = NULL;

    /* Register Tool with JSON Schema */
    tlc_tool_register(&tools,
                      "get_weather",
                      "Get current weather by city name",
                      "{\"type\":\"object\",\"properties\":{\"city\":{\"type\":\"string\"}},\"required\":[\"city\"]}",
                      get_weather_tool,
                      &tool_ctx);

    /* Initialize Agent */
    tlc_agent_init(&agent, "demo_agent", &runtime, &callbacks,
                   &memory, &model, &tools, NULL);

    /* Scenario A: Direct Inference */
    if (tlc_agent_run(&agent, "请简单介绍一下你自己", result, sizeof(result)) == TLC_OK) {
        printf("\n[AGENT RESULT 1]\n%s\n", result);
    }

    /* Scenario B: Tool Invocation Sequence */
    if (tlc_agent_run(&agent, "帮我查一下 weather", result, sizeof(result)) == TLC_OK) {
        printf("\n[AGENT RESULT 2]\n%s\n", result);
    }

    /* Scenario C: Linear Chain Composition */
    {
        tlc_prompt_template_t pt;
        tlc_chain_t chain;
        tlc_output_parser_t parser;
        tlc_memory_t chain_memory;
        char chain_out[2048];

        tlc_memory_init(&chain_memory);
        tlc_prompt_template_init(&pt, "simple_prompt", "请重写下面内容:\n{input}");
        
        memset(&parser, 0, sizeof(parser));
        tlc_strcpy_s(parser.name, sizeof(parser.name), "text_parser");
        parser.parse = tlc_text_output_parser;

        tlc_chain_init(&chain, "rewrite_chain", &runtime, &callbacks, &chain_memory, &tools);
        tlc_chain_add_prompt(&chain, &pt);
        tlc_chain_add_model(&chain, &model);
        tlc_chain_add_parser(&chain, &parser);

        if (tlc_chain_run(&chain, "今天心情很好", chain_out, sizeof(chain_out)) == TLC_OK) {
            printf("\n[CHAIN RESULT]\n%s\n", chain_out);
        }
    }

    return 0;
}
```

## 🛠 Build Instructions

The build system relies on standard POSIX `make`. The framework is structurally designed to possess **zero external dependencies** outside of the standard C library.

```bash
# Core framework compilation
make CFLAGS="-std=c11 -Wall -Wextra -Werror -O2 -Iinclude"

# Execution of the mock transport validation
./example_mock
```

To integrate into an RTOS project (e.g., FreeRTOS, Zephyr), link `libtidylangchain.a` statically and ensure `include/` is added to your compiler's include path.

## 📄 License

This software is distributed under the **Apache License 2.0**. See the [LICENSE](LICENSE) file for complete details.
