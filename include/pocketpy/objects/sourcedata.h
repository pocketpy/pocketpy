#pragma once

#include <stdbool.h>
#include "pocketpy/common/str.h"
#include "pocketpy/common/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

enum CompileMode { EXEC_MODE, EVAL_MODE, REPL_MODE, JSON_MODE, CELL_MODE };

struct pk_SourceData {
    RefCounted rc;
    enum CompileMode mode;
    bool is_precompiled;
    bool is_dynamic;    // for exec() and eval()

    py_Str filename;
    py_Str source;

    c11_vector/*T=const char* */ line_starts;
    c11_vector/*T=py_Str*/ _precompiled_tokens;
};

typedef struct pk_SourceData* pk_SourceData_;

pk_SourceData_ pk_SourceData__rcnew(const char* source, const char* filename, enum CompileMode mode, bool is_dynamic);
bool pk_SourceData__get_line(const struct pk_SourceData* self, int lineno, const char** st, const char** ed);
py_Str pk_SourceData__snapshot(const struct pk_SourceData *self, int lineno, const char *cursor, const char *name);

#ifdef __cplusplus
}
#endif
