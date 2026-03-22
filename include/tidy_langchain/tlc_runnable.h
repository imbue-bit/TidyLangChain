#ifndef TLC_RUNNABLE_H
#define TLC_RUNNABLE_H

#include "tlc_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlc_runnable_s tlc_runnable_t;

typedef tlc_status_t (*tlc_runnable_run_fn)(tlc_runnable_t* runnable,
                                            const char* input,
                                            char* output,
                                            size_t output_size);

struct tlc_runnable_s {
    char name[TLC_NAME_MAX];
    void* ctx;
    tlc_runnable_run_fn run;
};

#ifdef __cplusplus
}
#endif

#endif
