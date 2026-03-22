#ifndef TLC_STATUS_H
#define TLC_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum tlc_status_e {
    TLC_OK = 0,
    TLC_ERR_INVALID_ARG = -1,
    TLC_ERR_OVERFLOW = -2,
    TLC_ERR_NOT_FOUND = -3,
    TLC_ERR_UNSUPPORTED = -4,
    TLC_ERR_TRANSPORT = -5,
    TLC_ERR_HTTP = -6,
    TLC_ERR_JSON = -7,
    TLC_ERR_PARSE = -8,
    TLC_ERR_LIMIT = -9,
    TLC_ERR_INTERNAL = -10,
    TLC_ERR_EMPTY = -11,
    TLC_ERR_TOOL = -12,
    TLC_ERR_AGENT_STOP = -13
} tlc_status_t;

const char* tlc_status_str(tlc_status_t st);

#ifdef __cplusplus
}
#endif

#endif
