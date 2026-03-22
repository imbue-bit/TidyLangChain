#include "tidy_langchain/tlc_vectorstore.h"
#include "tidy_langchain/tlc_string.h"
#include <string.h>

static float tlc_dot(const float* a, const float* b, size_t n) {
    size_t i;
    float s = 0.0f;
    for (i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

void tlc_vectorstore_init(tlc_vectorstore_t* vs, tlc_embeddings_t* embeddings) {
    if (!vs) return;
    memset(vs, 0, sizeof(*vs));
    vs->embeddings = embeddings;
}

tlc_status_t tlc_vectorstore_add(tlc_vectorstore_t* vs,
                                 const char* id,
                                 const char* content,
                                 const char* metadata) {
    tlc_vector_item_t* item;
    if (!vs || !vs->embeddings || !id || !content) return TLC_ERR_INVALID_ARG;
    if (vs->count >= TLC_MAX_VECTOR_ITEMS) return TLC_ERR_LIMIT;
    if (vs->embeddings->dimension > TLC_MAX_VECTOR_DIM) return TLC_ERR_LIMIT;

    item = &vs->items[vs->count];
    memset(item, 0, sizeof(*item));
    item->used = true;
    item->dim = vs->embeddings->dimension;

    if (tlc_strcpy_s(item->doc.id, sizeof(item->doc.id), id) != TLC_OK) return TLC_ERR_OVERFLOW;
    if (tlc_strcpy_s(item->doc.content, sizeof(item->doc.content), content) != TLC_OK) return TLC_ERR_OVERFLOW;
    if (metadata && tlc_strcpy_s(item->doc.metadata, sizeof(item->doc.metadata), metadata) != TLC_OK) {
        return TLC_ERR_OVERFLOW;
    }

    if (vs->embeddings->embed(vs->embeddings, content, item->vec, item->dim) != TLC_OK) {
        return TLC_ERR_INTERNAL;
    }

    vs->count++;
    return TLC_OK;
}

tlc_status_t tlc_vectorstore_search(tlc_vectorstore_t* vs,
                                    const char* query,
                                    tlc_document_t* out_docs,
                                    size_t max_docs,
                                    size_t* out_count) {
    float qvec[TLC_MAX_VECTOR_DIM];
    size_t i, j;
    if (!vs || !vs->embeddings || !query || !out_docs || !out_count) return TLC_ERR_INVALID_ARG;
    if (vs->embeddings->dimension > TLC_MAX_VECTOR_DIM) return TLC_ERR_LIMIT;

    if (vs->embeddings->embed(vs->embeddings, query, qvec, vs->embeddings->dimension) != TLC_OK) {
        return TLC_ERR_INTERNAL;
    }

    *out_count = 0;
    for (i = 0; i < vs->count; ++i) {
        if (!vs->items[i].used) continue;

        {
            float score = tlc_dot(qvec, vs->items[i].vec, vs->items[i].dim);
            tlc_document_t d = vs->items[i].doc;
            d.score = score;

            if (*out_count < max_docs) {
                out_docs[*out_count] = d;
                (*out_count)++;
            } else {
                size_t min_idx = 0;
                for (j = 1; j < max_docs; ++j) {
                    if (out_docs[j].score < out_docs[min_idx].score) min_idx = j;
                }
                if (score > out_docs[min_idx].score) {
                    out_docs[min_idx] = d;
                }
            }
        }
    }

    for (i = 0; i < *out_count; ++i) {
        for (j = i + 1; j < *out_count; ++j) {
            if (out_docs[j].score > out_docs[i].score) {
                tlc_document_t tmp = out_docs[i];
                out_docs[i] = out_docs[j];
                out_docs[j] = tmp;
            }
        }
    }

    return TLC_OK;
}
