#ifndef TLC_TRANSPORT_H
#define TLC_TRANSPORT_H

#include "tlc_types.h"
#include "tlc_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlc_http_header_s {
    char key[TLC_MAX_HTTP_HEADER_KEY];
    char value[TLC_MAX_HTTP_HEADER_VALUE];
} tlc_http_header_t;

typedef struct tlc_http_request_s {
    char method[8];
    char host[TLC_MEDIUM_STR];
    uint16_t port;
    char path[TLC_LARGE_STR];
    bool use_tls;
    tlc_http_header_t headers[TLC_MAX_HTTP_HEADERS];
    size_t header_count;
    char body[TLC_MAX_HTTP_REQUEST_BODY];
} tlc_http_request_t;

typedef struct tlc_http_response_s {
    int status_code;
    tlc_http_header_t headers[TLC_MAX_HTTP_HEADERS];
    size_t header_count;
    char body[TLC_MAX_HTTP_RESPONSE_BODY];
} tlc_http_response_t;

typedef tlc_status_t (*tlc_http_send_fn)(void* user_ctx,
                                         const tlc_http_request_t* req,
                                         tlc_http_response_t* resp);

typedef struct tlc_transport_s {
    tlc_http_send_fn send;
    void* user_ctx;
} tlc_transport_t;

void tlc_http_request_init(tlc_http_request_t* req);
void tlc_http_response_init(tlc_http_response_t* resp);
tlc_status_t tlc_http_add_header(tlc_http_request_t* req,
                                 const char* key,
                                 const char* value);

#ifdef __cplusplus
}
#endif

#endif
