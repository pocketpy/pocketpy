#pragma once

#include "pocketpy/common/str.h"
#include "pocketpy/common/strname.h"
#include "pocketpy/objects/codeobject.h"
#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/pocketpy.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Error{
    const char* type;
    pk_SourceData_ src;
    int lineno;
    const char* cursor;
    char msg[100];
    int64_t userdata;
};

void py_BaseException__record(py_Ref, const Bytecode* ip, const CodeObject* code);
void py_BaseException__stpush(py_Ref, pk_SourceData_ src, int lineno, const char* func_name);

#ifdef __cplusplus
}
#endif