#pragma once

#include <stdbool.h>
#include "pocketpy/pocketpy.h"
#include "pocketpy/common/str.h"
#include "pocketpy/common/vector.h"

#ifdef __cplusplus
extern "C" {
#endif


struct pk_SourceData {
    RefCounted rc;
    enum py_CompileMode mode;
    bool is_precompiled;
    bool is_dynamic;    // for exec() and eval()

    c11_string* filename;
    c11_string* source;

    c11_vector/*T=const char* */ line_starts;
    c11_vector/*T=c11_string* */ _precompiled_tokens;
};

typedef struct pk_SourceData* pk_SourceData_;

pk_SourceData_ pk_SourceData__rcnew(const char* source, const char* filename, enum py_CompileMode mode, bool is_dynamic);
bool pk_SourceData__get_line(const struct pk_SourceData* self, int lineno, const char** st, const char** ed);
c11_string* pk_SourceData__snapshot(const struct pk_SourceData *self, int lineno, const char *cursor, const char *name);

#ifdef __cplusplus
}
#endif
