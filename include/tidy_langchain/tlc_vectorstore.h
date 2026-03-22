#ifndef TLC_VECTORSTORE_H
#define TLC_VECTORSTORE_H

#include "tlc_document.h"
#include "tlc_embeddings.h"
#include "tlc_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlc_vector_item_s {
    bool used;
    tlc_document_t doc;
    float vec[TLC_MAX_VECTOR_DIM];
    size_t dim;
} tlc_vector_item_t;

typedef struct tlc_vectorstore_s {
    tlc_embeddings_t* embeddings;
    tlc_vector_item_t items[TLC_MAX_VECTOR_ITEMS];
    size_t count;
} tlc_vectorstore_t;

void tlc_vectorstore_init(tlc_vectorstore_t* vs, tlc_embeddings_t* embeddings);
tlc_status_t tlc_vectorstore_add(tlc_vectorstore_t* vs,
                                 const char* id,
                                 const char* content,
                                 const char* metadata);

tlc_status_t tlc_vectorstore_search(tlc_vectorstore_t* vs,
                                    const char* query,
                                    tlc_document_t* out_docs,
                                    size_t max_docs,
                                    size_t* out_count);

#ifdef __cplusplus
}
#endif

#endif
