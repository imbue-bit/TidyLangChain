#ifndef TLC_TYPES_H
#define TLC_TYPES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "tlc_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlc_kv_s {
    char key[TLC_SMALL_STR];
    char value[TLC_MEDIUM_STR];
} tlc_kv_t;

typedef struct tlc_buffer_s {
    char* data;
    size_t size;
    size_t len;
} tlc_buffer_t;

typedef enum tlc_message_role_e {
    TLC_ROLE_SYSTEM = 0,
    TLC_ROLE_USER,
    TLC_ROLE_ASSISTANT,
    TLC_ROLE_TOOL
} tlc_message_role_t;

typedef enum tlc_chain_step_type_e {
    TLC_CHAIN_STEP_PROMPT = 0,
    TLC_CHAIN_STEP_MODEL,
    TLC_CHAIN_STEP_PARSER,
    TLC_CHAIN_STEP_TOOL,
    TLC_CHAIN_STEP_CUSTOM
} tlc_chain_step_type_t;

#ifdef __cplusplus
}
#endif

#endif
