#pragma once

#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/objects/base.h"

typedef struct BaseExceptionFrame {
    SourceData_ src;
    int lineno;
    c11_string* name;
    py_TValue locals;   // for debugger only
    py_TValue globals;  // for debugger only
} BaseExceptionFrame;

typedef struct BaseException {
    py_TValue args;
    py_TValue inner_exc;
    c11_vector /*T=BaseExceptionFrame*/ stacktrace;
} BaseException;

char* safe_stringify_exception(py_Ref exc);
char* formatexc_internal(py_Ref exc);