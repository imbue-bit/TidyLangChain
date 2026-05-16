# ⚛️ TidyLangChain

[English](REAMDE.md) | 简体中文

**资源受限环境下的 LLM Agent 编排确定性框架**

**[摘要](https://www.google.com/search?q=%23-%E6%91%98%E8%A6%81)** •
**[系统架构](https://www.google.com/search?q=%23-%E7%B3%BB%E7%BB%9F%E6%9E%B6%E6%9E%84)** •
**[实现范式](https://www.google.com/search?q=%23-%E5%AE%9E%E7%8E%B0%E8%8C%83%E5%BC%8F)** •
**[构建说明](https://www.google.com/search?q=%23-%E6%9E%84%E5%BB%BA%E8%AF%B4%E6%98%8E)**

## 📋 摘要

**TidyLangChain** 是一款以嵌入式为核心的软件框架，旨在简化微控制器（MCU）和边缘设备上大语言模型（LLM）及自主智能体（Agent）的编排。该框架完全采用严格的 ANSI C11 标准实现，将 LangChain 的认知编排原理转化为确定性、内存受限的执行环境。

通过利用预分配的内存预算和解耦的硬件抽象层（HAL）进行网络传输，TidyLangChain 保证了时间与空间上的确定性，使其非常适合需要片上智能、传感器/执行器工具调用以及多步推理的基于 RTOS 的物联网（IoT）设备。

## 🏛️ 系统架构

该框架在结构上划分为多个解耦的模块，以确保极小的体积和高度的可移植性：

* **运行时与内存 (`tlc_runtime`)**：采用连续内存池 (`tlc_runtime_init`) 来防止堆碎片化。所有内存分配都在严格预定义的内存预算内进行追踪。
* **硬件抽象层 (`tlc_transport`)**：网络操作完全通过函数指针进行抽象，从而允许与 LwIP、AT 指令序列或 POSIX 套接字进行无缝绑定。
* **认知编排 (`tlc_agent`, `tlc_chain`)**：实现顺序处理流水线（Chains）和反应式状态机（Agents），动态地将 LLM 输出映射到本地函数的执行。
* **外部工具注册表 (`tlc_tool`)**：提供标准化接口，用于定义可由 LLM 执行的、基于 JSON-schema 的硬件工具（例如：读取传感器、驱动 GPIO）。

## 💻 实现范式

以下代码展示了集成确定性 Agent 和线性 Chain 的完整生命周期，并利用模拟的 HTTP 传输层来模拟网络操作。

```c
#include "tidy_langchain/tidy_langchain.h"
#include <stdio.h>
#include <string.h>

/* ---------- 1. 回调与日志子系统 ---------- */
static void example_log(void* user, const char* event, const char* detail) {
    (void)user;
    printf("[LOG] %s: %s\n", event, detail ? detail : "");
}

/* ---------- 2. 传输抽象层 ---------- */
static tlc_status_t mock_send(void* user_ctx,
                              const tlc_http_request_t* req,
                              tlc_http_response_t* resp) {
    (void)user_ctx;
    printf("=== HTTP REQUEST ===\n");
    printf("%s https://%s:%u%s\n", req->method, req->host, req->port, req->path);
    printf("BODY: %s\n", req->body);

    resp->status_code = 200;

    /* 根据负载内容模拟生成响应 */
    if (strstr(req->body, "Based on tool results")) {
        tlc_strcpy_s(resp->body, sizeof(resp->body),
            "{\"id\":\"chatcmpl_mock\",\"choices\":[{\"message\":{"
            "\"role\":\"assistant\",\"content\":\"根据工具结果，上海当前天气晴，气温 26C。\"}}]}");
    } else {
        /* 用于触发工具的关键词检测 */
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
            /* 发送工具调用负载 */
            tlc_strcpy_s(resp->body, sizeof(resp->body),
                "{\"id\":\"chatcmpl_mock\",\"choices\":[{\"message\":{"
                "\"role\":\"assistant\",\"content\":\"\",\"tool_calls\":[{"
                "\"id\":\"call_1\",\"type\":\"function\",\"function\":{"
                "\"name\":\"get_weather\",\"arguments\":\"{\\\"city\\\":\\\"Shanghai\\\"}\"}}]}}]}");
        } else {
            /* 发送直接响应负载 */
            tlc_strcpy_s(resp->body, sizeof(resp->body),
                "{\"id\":\"chatcmpl_mock\",\"choices\":[{\"message\":{"
                "\"role\":\"assistant\",\"content\":\"这是一个直接回答，不需要调用工具。\"}}]}");
        }
    }
    return TLC_OK;
}

/* ---------- 3. 硬件工具定义 ---------- */
static tlc_status_t get_weather_tool(tlc_tool_ctx_t* ctx,
                                     const char* args_json,
                                     char* output,
                                     size_t output_size) {
    (void)ctx;
    (void)args_json;
    /* 在实际实现中，这里将读取 I2C/SPI 传感器 */
    return tlc_strcpy_s(output, output_size,
                        "{\"city\":\"Shanghai\",\"condition\":\"Sunny\",\"temp_c\":26}");
}

/* ---------- 4. 主执行与编排 ---------- */
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

    /* 使用确定性的 1MB 内存预算初始化运行时 */
    tlc_runtime_init(&runtime, 1024 * 1024);

    callbacks.on_log = example_log;
    callbacks.user = NULL;

    transport.send = mock_send;
    transport.user_ctx = NULL;

    tlc_chat_options_default(&options);
    tlc_strcpy_s(options.model, sizeof(options.model), "gpt-4o-mini");
    options.enable_tools = true;

    /* 初始化模型输入输出（I/O）上下文 */
    tlc_model_openai_init(&model, &runtime, &callbacks, &transport,
                          "api.openai.com", 443, true,
                          "/v1/chat/completions", "sk-demo", &options);

    tlc_memory_init(&memory);
    tlc_tool_registry_init(&tools);

    tool_ctx.runtime = &runtime;
    tool_ctx.callbacks = &callbacks;
    tool_ctx.user_data = NULL;

    /* 使用 JSON Schema 注册工具 */
    tlc_tool_register(&tools,
                      "get_weather",
                      "Get current weather by city name",
                      "{\"type\":\"object\",\"properties\":{\"city\":{\"type\":\"string\"}},\"required\":[\"city\"]}",
                      get_weather_tool,
                      &tool_ctx);

    /* 初始化 Agent */
    tlc_agent_init(&agent, "demo_agent", &runtime, &callbacks,
                   &memory, &model, &tools, NULL);

    /* 场景 A：直接推理 */
    if (tlc_agent_run(&agent, "请简单介绍一下你自己", result, sizeof(result)) == TLC_OK) {
        printf("\n[AGENT RESULT 1]\n%s\n", result);
    }

    /* 场景 B：工具调用序列 */
    if (tlc_agent_run(&agent, "帮我查一下 weather", result, sizeof(result)) == TLC_OK) {
        printf("\n[AGENT RESULT 2]\n%s\n", result);
    }

    /* 场景 C：线性链组合 */
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

## 🛠 构建说明

构建系统依赖于标准的 POSIX `make`。该框架在结构设计上对标准 C 库之外的外部依赖为零。

```bash
make CFLAGS="-std=c11 -Wall -Wextra -Werror -O2 -Iinclude"

./example_mock

```

若要集成到 RTOS 项目中（例如 FreeRTOS、Zephyr），请静态链接 `libtidylangchain.a` 并确保将 `include/` 添加到编译器的头文件搜索路径中。

## 📄 许可证

本软件基于 Apache License 2.0 许可证分发。完整细节请参见 [LICENSE]() 文件。
