#pragma once

#include "pocketpy/common/str.h"
#include "pocketpy/common/strname.h"
#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/objects/object.h"

#ifdef __cplusplus
extern "C" {
#endif

// typedef struct pkpy_ExceptionFrame {
//     pk_SourceData_ src;
//     int lineno;
//     const char* cursor;
//     c11_string* name;
// } pkpy_ExceptionFrame;

// typedef struct pkpy_Exception {
//     py_Name type;
//     c11_string* msg;
//     bool is_re;

//     int _ip_on_error;
//     void* _code_on_error;

//     PyObject* self;  // weak reference

//     c11_vector/*T=pkpy_ExceptionFrame*/ stacktrace;
// } pkpy_Exception;

// void pkpy_Exception__ctor(pkpy_Exception* self, py_Name type);
// void pkpy_Exception__dtor(pkpy_Exception* self);
// void pkpy_Exception__stpush(pkpy_Exception* self, pk_SourceData_ src, int lineno, const char* cursor, const char* name);
// py_Str pkpy_Exception__summary(pkpy_Exception* self);

struct Error{
    const char* type;
    pk_SourceData_ src;
    int lineno;
    const char* cursor;
    char msg[100];
    int64_t userdata;
};

#ifdef __cplusplus
}
#endif