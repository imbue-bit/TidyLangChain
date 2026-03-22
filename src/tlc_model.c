#include "tidy_langchain/tlc_model.h"
#include "tidy_langchain/tlc_string.h"
#include <string.h>

static tlc_status_t tlc_openai_build_body(const tlc_memory_t* memory,
                                          const char* prompt,
                                          const tlc_chat_options_t* opt,
                                          const tlc_tool_def_t* tools,
                                          size_t tool_count,
                                          char* out,
                                          size_t out_size) {
    tlc_buffer_t buf;
    char esc[TLC_MAX_MESSAGE_CONTENT * 2];
    size_t i;

    if (!prompt || !opt || !out || out_size == 0) return TLC_ERR_INVALID_ARG;
    tlc_buf_init(&buf, out, out_size);

    if (tlc_buf_append(&buf, "{") != TLC_OK) return TLC_ERR_OVERFLOW;

    if (tlc_buf_append(&buf, "\"model\":\"") != TLC_OK) return TLC_ERR_OVERFLOW;
    if (tlc_buf_append(&buf, opt->model) != TLC_OK) return TLC_ERR_OVERFLOW;
    if (tlc_buf_append(&buf, "\",") != TLC_OK) return TLC_ERR_OVERFLOW;

    {
        char tmp[64];
        if (tlc_snprintf(tmp, sizeof(tmp),
                         "\"temperature\":%.2f,\"max_tokens\":%u,",
                         (double)opt->temperature,
                         (unsigned)opt->max_tokens) != TLC_OK) {
            return TLC_ERR_OVERFLOW;
        }
        if (tlc_buf_append(&buf, tmp) != TLC_OK) return TLC_ERR_OVERFLOW;
    }

    if (tlc_buf_append(&buf, "\"messages\":[") != TLC_OK) return TLC_ERR_OVERFLOW;

    if (memory && memory->count > 0) {
        for (i = 0; i < memory->count; ++i) {
            const tlc_message_t* msg = tlc_memory_get(memory, i);
            if (!msg) continue;

            if (i > 0) {
                if (tlc_buf_append(&buf, ",") != TLC_OK) return TLC_ERR_OVERFLOW;
            }

            if (tlc_json_escape_into(msg->content, esc, sizeof(esc)) != TLC_OK) return TLC_ERR_OVERFLOW;
            if (tlc_buf_append(&buf, "{\"role\":\"") != TLC_OK) return TLC_ERR_OVERFLOW;
            if (tlc_buf_append(&buf, tlc_message_role_str(msg->role)) != TLC_OK) return TLC_ERR_OVERFLOW;
            if (tlc_buf_append(&buf, "\",") != TLC_OK) return TLC_ERR_OVERFLOW;

            if (msg->name[0]) {
                if (tlc_buf_append(&buf, "\"name\":\"") != TLC_OK) return TLC_ERR_OVERFLOW;
                if (tlc_buf_append(&buf, msg->name) != TLC_OK) return TLC_ERR_OVERFLOW;
                if (tlc_buf_append(&buf, "\",") != TLC_OK) return TLC_ERR_OVERFLOW;
            }

            if (tlc_buf_append(&buf, "\"content\":\"") != TLC_OK) return TLC_ERR_OVERFLOW;
            if (tlc_buf_append(&buf, esc) != TLC_OK) return TLC_ERR_OVERFLOW;
            if (tlc_buf_append(&buf, "\"}") != TLC_OK) return TLC_ERR_OVERFLOW;
        }

        if (tlc_buf_append(&buf, ",") != TLC_OK) return TLC_ERR_OVERFLOW;
    }

    if (tlc_json_escape_into(prompt, esc, sizeof(esc)) != TLC_OK) return TLC_ERR_OVERFLOW;
    if (tlc_buf_append(&buf, "{\"role\":\"user\",\"content\":\"") != TLC_OK) return TLC_ERR_OVERFLOW;
    if (tlc_buf_append(&buf, esc) != TLC_OK) return TLC_ERR_OVERFLOW;
    if (tlc_buf_append(&buf, "\"}]") != TLC_OK) return TLC_ERR_OVERFLOW;

    if (opt->enable_tools && tools && tool_count > 0) {
        if (tlc_buf_append(&buf, ",\"tools\":[") != TLC_OK) return TLC_ERR_OVERFLOW;
        for (i = 0; i < tool_count; ++i) {
            if (i > 0) {
                if (tlc_buf_append(&buf, ",") != TLC_OK) return TLC_ERR_OVERFLOW;
            }
            if (tlc_buf_append(&buf, "{\"type\":\"function\",\"function\":{") != TLC_OK) return TLC_ERR_OVERFLOW;
            if (tlc_buf_append(&buf, "\"name\":\"") != TLC_OK) return TLC_ERR_OVERFLOW;
            if (tlc_buf_append(&buf, tools[i].name) != TLC_OK) return TLC_ERR_OVERFLOW;
            if (tlc_buf_append(&buf, "\",\"description\":\"") != TLC_OK) return TLC_ERR_OVERFLOW;

            if (tlc_json_escape_into(tools[i].description, esc, sizeof(esc)) != TLC_OK) return TLC_ERR_OVERFLOW;
            if (tlc_buf_append(&buf, esc) != TLC_OK) return TLC_ERR_OVERFLOW;

            if (tlc_buf_append(&buf, "\",\"parameters\":") != TLC_OK) return TLC_ERR_OVERFLOW;
            if (tlc_buf_append(&buf, tools[i].json_schema[0] ? tools[i].json_schema : "{}") != TLC_OK) return TLC_ERR_OVERFLOW;
            if (tlc_buf_append(&buf, "}}") != TLC_OK) return TLC_ERR_OVERFLOW;
        }
        if (tlc_buf_append(&buf, "]") != TLC_OK) return TLC_ERR_OVERFLOW;
    }

    if (tlc_buf_append(&buf, "}") != TLC_OK) return TLC_ERR_OVERFLOW;
    return TLC_OK;
}

void tlc_chat_options_default(tlc_chat_options_t* opt) {
    if (!opt) return;
    memset(opt, 0, sizeof(*opt));
    tlc_strcpy_s(opt->model, sizeof(opt->model), "gpt-4o-mini");
    opt->temperature = 0.2f;
    opt->max_tokens = 512;
    opt->enable_tools = false;
}

tlc_status_t tlc_model_openai_init(tlc_model_t* model,
                                   tlc_runtime_t* runtime,
                                   tlc_callbacks_t* callbacks,
                                   const tlc_transport_t* transport,
                                   const char* host,
                                   uint16_t port,
                                   bool use_tls,
                                   const char* path,
                                   const char* api_key,
                                   const tlc_chat_options_t* options) {
    if (!model || !runtime || !transport || !transport->send || !host || !api_key) {
        return TLC_ERR_INVALID_ARG;
    }

    memset(model, 0, sizeof(*model));
    tlc_strcpy_s(model->name, sizeof(model->name), "openai_chat_model");
    model->runtime = runtime;
    model->callbacks = callbacks;
    model->transport = *transport;
    model->port = port;
    model->use_tls = use_tls;
    model->invoke = tlc_model_openai_invoke;

    if (tlc_strcpy_s(model->host, sizeof(model->host), host) != TLC_OK) return TLC_ERR_OVERFLOW;
    if (tlc_strcpy_s(model->api_key, sizeof(model->api_key), api_key) != TLC_OK) return TLC_ERR_OVERFLOW;
    if (tlc_strcpy_s(model->path, sizeof(model->path), path ? path : TLC_DEFAULT_OPENAI_CHAT_PATH) != TLC_OK) {
        return TLC_ERR_OVERFLOW;
    }

    if (options) model->options = *options;
    else tlc_chat_options_default(&model->options);

    return TLC_OK;
}

void tlc_http_request_init(tlc_http_request_t* req) {
    if (!req) return;
    memset(req, 0, sizeof(*req));
    tlc_strcpy_s(req->method, sizeof(req->method), "POST");
}

void tlc_http_response_init(tlc_http_response_t* resp) {
    if (!resp) return;
    memset(resp, 0, sizeof(*resp));
}

tlc_status_t tlc_http_add_header(tlc_http_request_t* req,
                                 const char* key,
                                 const char* value) {
    tlc_http_header_t* h;
    if (!req || !key || !value) return TLC_ERR_INVALID_ARG;
    if (req->header_count >= TLC_MAX_HTTP_HEADERS) return TLC_ERR_LIMIT;
    h = &req->headers[req->header_count++];
    if (tlc_strcpy_s(h->key, sizeof(h->key), key) != TLC_OK) return TLC_ERR_OVERFLOW;
    if (tlc_strcpy_s(h->value, sizeof(h->value), value) != TLC_OK) return TLC_ERR_OVERFLOW;
    return TLC_OK;
}

tlc_status_t tlc_model_openai_invoke(tlc_model_t* model,
                                     const tlc_memory_t* memory,
                                     const char* prompt,
                                     const tlc_tool_def_t* tools,
                                     size_t tool_count,
                                     char* out,
                                     size_t out_size) {
    tlc_http_request_t req;
    tlc_http_response_t resp;
    char auth[TLC_LARGE_STR + 16];

    if (!model || !prompt || !out || out_size == 0) return TLC_ERR_INVALID_ARG;
    if (!model->transport.send) return TLC_ERR_TRANSPORT;

    tlc_callback_log(model->callbacks, "model.invoke.begin", prompt);

    if (tlc_runtime_reserve(model->runtime, sizeof(req) + sizeof(resp)) != TLC_OK) {
        return TLC_ERR_LIMIT;
    }

    tlc_http_request_init(&req);
    tlc_http_response_init(&resp);

    if (tlc_strcpy_s(req.host, sizeof(req.host), model->host) != TLC_OK) goto fail_overflow;
    if (tlc_strcpy_s(req.path, sizeof(req.path), model->path) != TLC_OK) goto fail_overflow;
    req.port = model->port;
    req.use_tls = model->use_tls;

    if (tlc_http_add_header(&req, "Content-Type", "application/json") != TLC_OK) goto fail_overflow;
    if (tlc_snprintf(auth, sizeof(auth), "Bearer %s", model->api_key) != TLC_OK) goto fail_overflow;
    if (tlc_http_add_header(&req, "Authorization", auth) != TLC_OK) goto fail_overflow;

    if (tlc_openai_build_body(memory,
                              prompt,
                              &model->options,
                              tools,
                              tool_count,
                              req.body,
                              sizeof(req.body)) != TLC_OK) {
        goto fail_overflow;
    }

    if (model->transport.send(model->transport.user_ctx, &req, &resp) != TLC_OK) {
        tlc_runtime_release(model->runtime, sizeof(req) + sizeof(resp));
        tlc_callback_log(model->callbacks, "model.invoke.transport_error", "");
        return TLC_ERR_TRANSPORT;
    }

    if (resp.status_code < 200 || resp.status_code >= 300) {
        tlc_runtime_release(model->runtime, sizeof(req) + sizeof(resp));
        tlc_callback_log(model->callbacks, "model.invoke.http_error", resp.body);
        return TLC_ERR_HTTP;
    }

    if (tlc_strcpy_s(out, out_size, resp.body) != TLC_OK) {
        tlc_runtime_release(model->runtime, sizeof(req) + sizeof(resp));
        return TLC_ERR_OVERFLOW;
    }

    tlc_runtime_release(model->runtime, sizeof(req) + sizeof(resp));
    tlc_callback_log(model->callbacks, "model.invoke.end", "ok");
    return TLC_OK;

fail_overflow:
    tlc_runtime_release(model->runtime, sizeof(req) + sizeof(resp));
    return TLC_ERR_OVERFLOW;
}

tlc_status_t tlc_model_extract_first_tool_call(const char* model_json,
                                               char* tool_name,
                                               size_t tool_name_size,
                                               char* tool_args_json,
                                               size_t tool_args_size) {
    tlc_status_t s1, s2;
    if (!model_json || !tool_name || !tool_args_json) return TLC_ERR_INVALID_ARG;
    s1 = tlc_json_get_openai_first_tool_name(model_json, tool_name, tool_name_size);
    s2 = tlc_json_get_openai_first_tool_args(model_json, tool_args_json, tool_args_size);
    if (s1 != TLC_OK || s2 != TLC_OK) return TLC_ERR_NOT_FOUND;
    return TLC_OK;
}
