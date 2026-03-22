#include "tidy_langchain/tlc_memory.h"
#include "tidy_langchain/tlc_string.h"
#include <string.h>

void tlc_memory_init(tlc_memory_t* mem) {
    if (!mem) return;
    memset(mem, 0, sizeof(*mem));
}

static size_t tlc_memory_index(const tlc_memory_t* mem, size_t logical_idx) {
    return (mem->start + logical_idx) % TLC_MAX_MESSAGES;
}

tlc_status_t tlc_memory_add(tlc_memory_t* mem,
                            tlc_message_role_t role,
                            const char* name,
                            const char* content) {
    size_t idx;
    if (!mem || !content) return TLC_ERR_INVALID_ARG;

    if (mem->count < TLC_MAX_MESSAGES) {
        idx = tlc_memory_index(mem, mem->count);
        mem->count++;
    } else {
        idx = mem->start;
        mem->start = (mem->start + 1) % TLC_MAX_MESSAGES;
    }

    return tlc_message_init(&mem->messages[idx], role, name, content);
}

size_t tlc_memory_count(const tlc_memory_t* mem) {
    if (!mem) return 0;
    return mem->count;
}

const tlc_message_t* tlc_memory_get(const tlc_memory_t* mem, size_t idx) {
    if (!mem || idx >= mem->count) return NULL;
    return &mem->messages[tlc_memory_index(mem, idx)];
}

tlc_status_t tlc_memory_render_chat(const tlc_memory_t* mem,
                                    char* out,
                                    size_t out_size) {
    tlc_buffer_t buf;
    size_t i;
    if (!mem || !out || out_size == 0) return TLC_ERR_INVALID_ARG;

    tlc_buf_init(&buf, out, out_size);

    for (i = 0; i < mem->count; ++i) {
        const tlc_message_t* msg = tlc_memory_get(mem, i);
        if (!msg) continue;
        if (tlc_buf_append(&buf, "[") != TLC_OK) return TLC_ERR_OVERFLOW;
        if (tlc_buf_append(&buf, tlc_message_role_str(msg->role)) != TLC_OK) return TLC_ERR_OVERFLOW;
        if (msg->name[0]) {
            if (tlc_buf_append(&buf, ":") != TLC_OK) return TLC_ERR_OVERFLOW;
            if (tlc_buf_append(&buf, msg->name) != TLC_OK) return TLC_ERR_OVERFLOW;
        }
        if (tlc_buf_append(&buf, "] ") != TLC_OK) return TLC_ERR_OVERFLOW;
        if (tlc_buf_append(&buf, msg->content) != TLC_OK) return TLC_ERR_OVERFLOW;
        if (tlc_buf_append(&buf, "\n") != TLC_OK) return TLC_ERR_OVERFLOW;
    }

    return TLC_OK;
}

void tlc_memory_clear(tlc_memory_t* mem) {
    if (!mem) return;
    memset(mem, 0, sizeof(*mem));
}
