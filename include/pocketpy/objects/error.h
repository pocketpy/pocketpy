#pragma once

#include "pocketpy/common/str.h"
#include "pocketpy/common/strname.h"
#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/objects/object.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pkpy_ExceptionFrame {
    pkpy_SourceData_ src;
    int lineno;
    const char* cursor;
    pkpy_Str name;
} pkpy_ExceptionFrame;

typedef struct pkpy_Exception {
    StrName type;
    pkpy_Str msg;
    bool is_re;

    int _ip_on_error;
    void* _code_on_error;

    PyObject* self;  // weak reference

    c11_vector/*T=pkpy_ExceptionFrame*/ stacktrace;
} pkpy_Exception;

void pkpy_Exception__ctor(pkpy_Exception* self, StrName type);
void pkpy_Exception__dtor(pkpy_Exception* self);
void pkpy_Exception__stpush(pkpy_Exception* self, pkpy_SourceData_ src, int lineno, const char* cursor, const char* name);
pkpy_Str pkpy_Exception__summary(pkpy_Exception* self);

struct Error{
    const char* type;
    pkpy_SourceData_ src;
    int lineno;
    const char* cursor;
    char msg[100];
    int64_t userdata;
};

#ifdef __cplusplus
}
#endif