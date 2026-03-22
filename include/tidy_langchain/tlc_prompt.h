#ifndef TLC_PROMPT_H
#define TLC_PROMPT_H

#include "tlc_types.h"
#include "tlc_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlc_prompt_var_s {
    char key[TLC_SMALL_STR];
    char value[TLC_MAX_MESSAGE_CONTENT];
} tlc_prompt_var_t;

typedef struct tlc_prompt_template_s {
    char name[TLC_NAME_MAX];
    char tpl[TLC_MAX_PROMPT_TEMPLATE];
} tlc_prompt_template_t;

tlc_status_t tlc_prompt_template_init(tlc_prompt_template_t* pt,
                                      const char* name,
                                      const char* tpl);

tlc_status_t tlc_prompt_format(const tlc_prompt_template_t* pt,
                               const tlc_prompt_var_t* vars,
                               size_t var_count,
                               char* out,
                               size_t out_size);

#ifdef __cplusplus
}
#endif

#endif
