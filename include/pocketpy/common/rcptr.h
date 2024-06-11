#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "pocketpy/common/config.h"

typedef struct {
#if PK_DEBUG_DATASTRUCTURE
    unsigned int magic;
#endif
    unsigned int ref_c;
    void (*dtor)(void *self);
} pkpy_Rcptr_header;

void pkpy_Rcptr__ctor(void *self);
void pkpy_Rcptr__ctor_withd(void *self, void *dtor);
void pkpy_Rcptr__ref(void *self);
void pkpy_Rcptr__unref(void *self);

#ifdef __cplusplus
}
#endif
