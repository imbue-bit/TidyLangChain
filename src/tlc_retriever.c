#include "tidy_langchain/tlc_retriever.h"
#include "tidy_langchain/tlc_string.h"
#include <string.h>

void tlc_vectorstore_retriever_init(tlc_retriever_t* retriever,
                                    tlc_vectorstore_t* vs) {
    if (!retriever) return;
    memset(retriever, 0, sizeof(*retriever));
    tlc_strcpy_s(retriever->name, sizeof(retriever->name), "vectorstore_retriever");
    retriever->ctx = vs;
    retriever->retrieve = tlc_vectorstore_retrieve;
}

tlc_status_t tlc_vectorstore_retrieve(tlc_retriever_t* retriever,
                                      const char* query,
                                      tlc_document_t* out_docs,
                                      size_t max_docs,
                                      size_t* out_count) {
    tlc_vectorstore_t* vs;
    if (!retriever || !query || !out_docs || !out_count) return TLC_ERR_INVALID_ARG;
    vs = (tlc_vectorstore_t*)retriever->ctx;
    if (!vs) return TLC_ERR_INVALID_ARG;
    return tlc_vectorstore_search(vs, query, out_docs, max_docs, out_count);
}
