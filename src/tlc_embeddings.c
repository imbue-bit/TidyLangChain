#include "tidy_langchain/tlc_embeddings.h"
#include "tidy_langchain/tlc_string.h"
#include <string.h>

void tlc_hash_embeddings_init(tlc_embeddings_t* emb,
                              tlc_runtime_t* runtime,
                              tlc_callbacks_t* callbacks,
                              size_t dimension) {
    if (!emb) return;
    memset(emb, 0, sizeof(*emb));
    tlc_strcpy_s(emb->name, sizeof(emb->name), "hash_embeddings");
    emb->runtime = runtime;
    emb->callbacks = callbacks;
    emb->dimension = dimension;
    emb->embed = tlc_hash_embeddings_embed;
}

tlc_status_t tlc_hash_embeddings_embed(tlc_embeddings_t* emb,
                                       const char* text,
                                       float* out_vec,
                                       size_t dim) {
    size_t i;
    if (!emb || !text || !out_vec || dim == 0) return TLC_ERR_INVALID_ARG;
    if (dim > TLC_MAX_VECTOR_DIM) return TLC_ERR_LIMIT;

    for (i = 0; i < dim; ++i) out_vec[i] = 0.0f;

    for (i = 0; text[i] != '\0'; ++i) {
        unsigned char c = (unsigned char)text[i];
        size_t idx = ((size_t)c + i * 131u) % dim;
        out_vec[idx] += 1.0f;
    }

    return TLC_OK;
}
