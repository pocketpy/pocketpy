#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "pocketpy/common/str.h"
#include "pocketpy/common/vector.h"

enum CompileMode { EXEC_MODE, EVAL_MODE, REPL_MODE, JSON_MODE, CELL_MODE };

struct pkpy_SourceData {
    enum CompileMode mode;
    bool is_precompiled;

    pkpy_Str filename;
    pkpy_Str source;

    c11_vector line_starts;     // contains "const char *"
    c11_vector _precompiled_tokens;  // contains "pkpy_Str"
};

void pkpy_SourceData__ctor(struct pkpy_SourceData *self, const char *source, int source_size, const pkpy_Str *filename, enum CompileMode mode);
void pkpy_SourceData__dtor(struct pkpy_SourceData* self);

bool pkpy_SourceData__get_line(const struct pkpy_SourceData *self, int lineno, const char **st, const char **ed);
pkpy_Str pkpy_SourceData__snapshot(const struct pkpy_SourceData *self, int lineno, const char *cursor, const char *name);

#ifdef __cplusplus
}
#endif
