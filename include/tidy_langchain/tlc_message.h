#ifndef TLC_MESSAGE_H
#define TLC_MESSAGE_H

#include "tlc_types.h"
#include "tlc_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlc_message_s {
    tlc_message_role_t role;
    char name[TLC_NAME_MAX];
    char content[TLC_MAX_MESSAGE_CONTENT];
} tlc_message_t;

const char* tlc_message_role_str(tlc_message_role_t role);
tlc_status_t tlc_message_init(tlc_message_t* msg,
                              tlc_message_role_t role,
                              const char* name,
                              const char* content);

#ifdef __cplusplus
}
#endif

#endif
