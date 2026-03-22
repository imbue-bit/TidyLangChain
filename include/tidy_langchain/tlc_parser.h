#ifndef TLC_PARSER_H
#define TLC_PARSER_H

#include "tlc_status.h"
#include "tlc_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlc_output_parser_s tlc_output_parser_t;
typedef tlc_status_t (*tlc_output_parse_fn)(tlc_output_parser_t* parser,
                                            const char* input,
                                            char* output,
                                            size_t output_size);

struct tlc_output_parser_s {
    char name[TLC_NAME_MAX];
    void* ctx;
    tlc_output_parse_fn parse;
};

tlc_status_t tlc_text_output_parser(tlc_output_parser_t* parser,
                                    const char* input,
                                    char* output,
                                    size_t output_size);

tlc_status_t tlc_json_field_output_parser(tlc_output_parser_t* parser,
                                          const char* input,
                                          char* output,
                                          size_t output_size);

#ifdef __cplusplus
}
#endif

#endif
