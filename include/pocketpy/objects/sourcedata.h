#pragma once

#include <stdbool.h>
#include "pocketpy/common/str.h"
#include "pocketpy/common/vector.h"
#include "pocketpy/common/refcount.h"

#ifdef __cplusplus
extern "C" {
#endif

enum CompileMode { EXEC_MODE, EVAL_MODE, REPL_MODE, JSON_MODE, CELL_MODE };

struct pkpy_SourceData {
    RefCounted rc;
    enum CompileMode mode;
    bool is_precompiled;

    py_Str filename;
    py_Str source;

    c11_vector/*T=const char* */ line_starts;
    c11_vector/*T=py_Str*/ _precompiled_tokens;
};

typedef struct pkpy_SourceData* pkpy_SourceData_;

pkpy_SourceData_ pkpy_SourceData__rcnew(c11_string source, const py_Str *filename, enum CompileMode mode);
void pkpy_SourceData__ctor(struct pkpy_SourceData *self, c11_string source, const py_Str *filename, enum CompileMode mode);
void pkpy_SourceData__dtor(struct pkpy_SourceData* self);

bool pkpy_SourceData__get_line(const struct pkpy_SourceData* self, int lineno, const char** st, const char** ed);
py_Str pkpy_SourceData__snapshot(const struct pkpy_SourceData *self, int lineno, const char *cursor, const char *name);

#ifdef __cplusplus
}
#endif
