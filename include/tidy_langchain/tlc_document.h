#ifndef TLC_DOCUMENT_H
#define TLC_DOCUMENT_H

#include "tlc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlc_document_s {
    char id[TLC_SMALL_STR];
    char content[TLC_MAX_DOCUMENT_CONTENT];
    char metadata[TLC_MAX_METADATA];
    float score;
} tlc_document_t;

#ifdef __cplusplus
}
#endif

#endif
