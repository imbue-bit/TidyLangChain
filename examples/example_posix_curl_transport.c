#include "tidy_langchain/tidy_langchain.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void example_log(void* user, const char* event, const char* detail) {
    (void)user;
    printf("[LOG] %s: %s\n", event, detail ? detail : "");
}

static tlc_status_t curl_send(void* user_ctx,
                              const tlc_http_request_t* req,
                              tlc_http_response_t* resp) {
    char cmd[32768];
    char tmpfile[128] = "tlc_resp_tmp.json";
    FILE* fp;
    size_t n;

    (void)user_ctx;

    if (snprintf(cmd, sizeof(cmd),
                 "curl -s -X %s %s://%s:%u%s "
                 "-H \"Content-Type: application/json\" "
                 "-H \"Authorization: %s\" "
                 "-d '%s' -o %s",
                 req->method,
                 req->use_tls ? "https" : "http",
                 req->host,
                 req->port,
                 req->path,
                 req->headers[1].value,
                 req->body,
                 tmpfile) >= (int)sizeof(cmd)) {
        return TLC_ERR_OVERFLOW;
    }

    if (system(cmd) != 0) return TLC_ERR_TRANSPORT;

    fp = fopen(tmpfile, "rb");
    if (!fp) return TLC_ERR_TRANSPORT;

    n = fread(resp->body, 1, sizeof(resp->body) - 1, fp);
    fclose(fp);
    resp->body[n] = '\0';
    resp->status_code = 200;

    remove(tmpfile);
    return TLC_OK;
}

static tlc_status_t echo_tool(tlc_tool_ctx_t* ctx,
                              const char* args_json,
                              char* output,
                              size_t output_size) {
    (void)ctx;
    return tlc_snprintf(output, output_size, "{\"echo\":%s}", args_json ? args_json : "\"\"");
}

int main(void) {
    const char* api_key = getenv("OPENAI_API_KEY");
    tlc_runtime_t runtime;
    tlc_callbacks_t callbacks;
    tlc_transport_t transport;
    tlc_chat_options_t options;
    tlc_model_t model;
    tlc_memory_t memory;
    tlc_tool_registry_t tools;
    tlc_agent_t agent;
    tlc_tool_ctx_t tool_ctx;
    char result[4096];

    if (!api_key) {
        printf("Please export OPENAI_API_KEY first.\n");
        return 1;
    }

    tlc_runtime_init(&runtime, 1024 * 1024);
    callbacks.on_log = example_log;
    callbacks.user = NULL;

    transport.send = curl_send;
    transport.user_ctx = NULL;

    tlc_chat_options_default(&options);
    options.enable_tools = true;

    if (tlc_model_openai_init(&model,
                              &runtime,
                              &callbacks,
                              &transport,
                              "api.openai.com",
                              443,
                              true,
                              "/v1/chat/completions",
                              api_key,
                              &options) != TLC_OK) {
        printf("model init failed\n");
        return 1;
    }

    tlc_memory_init(&memory);
    tlc_tool_registry_init(&tools);

    tool_ctx.runtime = &runtime;
    tool_ctx.callbacks = &callbacks;
    tool_ctx.user_data = NULL;

    tlc_tool_register(&tools,
                      "echo_json",
                      "Echo input json back to caller",
                      "{\"type\":\"object\",\"properties\":{\"text\":{\"type\":\"string\"}},\"required\":[\"text\"]}",
                      echo_tool,
                      &tool_ctx);

    tlc_agent_init(&agent, "posix_agent", &runtime, &callbacks, &memory, &model, &tools, NULL);

    if (tlc_agent_run(&agent, "Say hello briefly.", result, sizeof(result)) == TLC_OK) {
        printf("%s\n", result);
    } else {
        printf("agent run failed\n");
    }

    return 0;
}
