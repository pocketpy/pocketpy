#include "pocketpy/common/rcptr.h"
#include <assert.h>
#include <stdlib.h>

#define RCPTR_MAGIC 0x3c3d3e3f

#if PK_DEBUG_DATASTRUCTURE
#define CHECK_MAGIC() assert(self->magic == RCPTR_MAGIC)
#else
#define CHECK_MAGIC() while(0)
#endif

void pkpy_Rcptr__ctor(void *self) {
    pkpy_Rcptr__ctor_withd(self, NULL);
}

void pkpy_Rcptr__ctor_withd(void *self_, void (*dtor)(void *)) {
    pkpy_Rcptr_header *self = self_;
#if PK_DEBUG_DATASTRUCTURE
    self->magic = RCPTR_MAGIC;
#endif
    self->ref_c = 1;
    self->dtor = dtor;
}

void pkpy_Rcptr__ref(void *self_) {
    if (self_ == NULL)
        return;
    pkpy_Rcptr_header *self = self_;
    CHECK_MAGIC();
    self->ref_c += 1;
}

void pkpy_Rcptr__unref(void *self_) {
    if (self_ == NULL)
        return;
    pkpy_Rcptr_header *self = self_;
    CHECK_MAGIC();
    self->ref_c -= 1;
    if (self->ref_c == 0) {
        if (self->dtor)
            self->dtor(self_);
        free(self_);
    }
}
