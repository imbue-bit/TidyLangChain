#include "tidy_langchain/tlc_message.h"
#include "tidy_langchain/tlc_string.h"

const char* tlc_message_role_str(tlc_message_role_t role) {
    switch (role) {
        case TLC_ROLE_SYSTEM: return "system";
        case TLC_ROLE_USER: return "user";
        case TLC_ROLE_ASSISTANT: return "assistant";
        case TLC_ROLE_TOOL: return "tool";
        default: return "user";
    }
}

tlc_status_t tlc_message_init(tlc_message_t* msg,
                              tlc_message_role_t role,
                              const char* name,
                              const char* content) {
    if (!msg || !content) return TLC_ERR_INVALID_ARG;
    msg->role = role;
    msg->name[0] = '\0';
    msg->content[0] = '\0';

    if (name) {
        if (tlc_strcpy_s(msg->name, sizeof(msg->name), name) != TLC_OK) {
            return TLC_ERR_OVERFLOW;
        }
    }
    if (tlc_strcpy_s(msg->content, sizeof(msg->content), content) != TLC_OK) {
        return TLC_ERR_OVERFLOW;
    }
    return TLC_OK;
}
