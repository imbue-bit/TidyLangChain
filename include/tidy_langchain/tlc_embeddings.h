#ifndef TLC_EMBEDDINGS_H
#define TLC_EMBEDDINGS_H

#include "tlc_status.h"
#include "tlc_runtime.h"
#include "tlc_callbacks.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlc_embeddings_s tlc_embeddings_t;

typedef tlc_status_t (*tlc_embed_fn)(tlc_embeddings_t* emb,
                                     const char* text,
                                     float* out_vec,
                                     size_t dim);

struct tlc_embeddings_s {
    char name[TLC_NAME_MAX];
    tlc_runtime_t* runtime;
    tlc_callbacks_t* callbacks;
    size_t dimension;
    void* ctx;
    tlc_embed_fn embed;
};

tlc_status_t tlc_hash_embeddings_embed(tlc_embeddings_t* emb,
                                       const char* text,
                                       float* out_vec,
                                       size_t dim);

void tlc_hash_embeddings_init(tlc_embeddings_t* emb,
                              tlc_runtime_t* runtime,
                              tlc_callbacks_t* callbacks,
                              size_t dimension);

#ifdef __cplusplus
}
#endif

#endif
