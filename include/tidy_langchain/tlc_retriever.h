#ifndef TLC_RETRIEVER_H
#define TLC_RETRIEVER_H

#include "tlc_vectorstore.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlc_retriever_s tlc_retriever_t;

typedef tlc_status_t (*tlc_retrieve_fn)(tlc_retriever_t* retriever,
                                        const char* query,
                                        tlc_document_t* out_docs,
                                        size_t max_docs,
                                        size_t* out_count);

struct tlc_retriever_s {
    char name[TLC_NAME_MAX];
    void* ctx;
    tlc_retrieve_fn retrieve;
};

void tlc_vectorstore_retriever_init(tlc_retriever_t* retriever,
                                    tlc_vectorstore_t* vs);

tlc_status_t tlc_vectorstore_retrieve(tlc_retriever_t* retriever,
                                      const char* query,
                                      tlc_document_t* out_docs,
                                      size_t max_docs,
                                      size_t* out_count);

#ifdef __cplusplus
}
#endif

#endif
