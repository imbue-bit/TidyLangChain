#include "tidy_langchain/tlc_prompt.h"
#include "tidy_langchain/tlc_string.h"
#include <string.h>

tlc_status_t tlc_prompt_template_init(tlc_prompt_template_t* pt,
                                      const char* name,
                                      const char* tpl) {
    if (!pt || !name || !tpl) return TLC_ERR_INVALID_ARG;
    if (tlc_strcpy_s(pt->name, sizeof(pt->name), name) != TLC_OK) return TLC_ERR_OVERFLOW;
    if (tlc_strcpy_s(pt->tpl, sizeof(pt->tpl), tpl) != TLC_OK) return TLC_ERR_OVERFLOW;
    return TLC_OK;
}

static const char* tlc_find_var(const tlc_prompt_var_t* vars,
                                size_t var_count,
                                const char* key) {
    size_t i;
    for (i = 0; i < var_count; ++i) {
        if (tlc_streq(vars[i].key, key)) return vars[i].value;
    }
    return NULL;
}

tlc_status_t tlc_prompt_format(const tlc_prompt_template_t* pt,
                               const tlc_prompt_var_t* vars,
                               size_t var_count,
                               char* out,
                               size_t out_size) {
    tlc_buffer_t buf;
    size_t i = 0;

    if (!pt || !out || out_size == 0) return TLC_ERR_INVALID_ARG;
    tlc_buf_init(&buf, out, out_size);

    while (pt->tpl[i] != '\0') {
        if (pt->tpl[i] == '{') {
            size_t j = i + 1;
            char key[TLC_SMALL_STR];
            size_t k = 0;
            while (pt->tpl[j] != '\0' && pt->tpl[j] != '}') {
                if (k + 1 >= sizeof(key)) return TLC_ERR_OVERFLOW;
                key[k++] = pt->tpl[j++];
            }
            if (pt->tpl[j] != '}') return TLC_ERR_PARSE;
            key[k] = '\0';

            {
                const char* v = tlc_find_var(vars, var_count, key);
                if (!v) return TLC_ERR_NOT_FOUND;
                if (tlc_buf_append(&buf, v) != TLC_OK) return TLC_ERR_OVERFLOW;
            }
            i = j + 1;
            continue;
        }

        if (tlc_buf_append_char(&buf, pt->tpl[i]) != TLC_OK) return TLC_ERR_OVERFLOW;
        i++;
    }

    return TLC_OK;
}
