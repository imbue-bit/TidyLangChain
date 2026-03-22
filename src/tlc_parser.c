#include <stddef.h>
#include "tidy_langchain/tlc_parser.h"
#include "tidy_langchain/tlc_json.h"
#include "tidy_langchain/tlc_string.h"

tlc_status_t tlc_text_output_parser(tlc_output_parser_t* parser,
                                    const char* input,
                                    char* output,
                                    size_t output_size) {
    (void)parser;
    if (!input || !output || output_size == 0) return TLC_ERR_INVALID_ARG;
    return tlc_strcpy_s(output, output_size, input);
}

tlc_status_t tlc_json_field_output_parser(tlc_output_parser_t* parser,
                                          const char* input,
                                          char* output,
                                          size_t output_size) {
    const char* field;
    if (!parser || !input || !output || output_size == 0) return TLC_ERR_INVALID_ARG;
    field = (const char*)parser->ctx;
    if (!field) return TLC_ERR_INVALID_ARG;
    return tlc_json_get_string(input, field, output, output_size);
}
