#include "tidy_langchain/tlc_callbacks.h"

void tlc_callback_log(const tlc_callbacks_t* cb,
                      const char* event,
                      const char* detail) {
    if (!cb || !cb->on_log) return;
    cb->on_log(cb->user, event, detail);
}