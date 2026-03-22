#ifndef TLC_MEMORY_H
#define TLC_MEMORY_H

#include "tlc_message.h"
#include "tlc_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlc_memory_s {
    tlc_message_t messages[TLC_MAX_MESSAGES];
    size_t count;
    size_t start;
} tlc_memory_t;

void tlc_memory_init(tlc_memory_t* mem);
tlc_status_t tlc_memory_add(tlc_memory_t* mem,
                            tlc_message_role_t role,
                            const char* name,
                            const char* content);
size_t tlc_memory_count(const tlc_memory_t* mem);
const tlc_message_t* tlc_memory_get(const tlc_memory_t* mem, size_t idx);
tlc_status_t tlc_memory_render_chat(const tlc_memory_t* mem,
                                    char* out,
                                    size_t out_size);
void tlc_memory_clear(tlc_memory_t* mem);

#ifdef __cplusplus
}
#endif

#endif
