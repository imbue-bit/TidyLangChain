#ifndef TLC_CALLBACKS_H
#define TLC_CALLBACKS_H

#include "tlc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*tlc_on_log_fn)(void* user, const char* event, const char* detail);

typedef struct tlc_callbacks_s {
    tlc_on_log_fn on_log;
    void* user;
} tlc_callbacks_t;

void tlc_callback_log(const tlc_callbacks_t* cb, const char* event, const char* detail);

#ifdef __cplusplus
}
#endif

#endif
