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

typedef struct{
    pk_SourceData_ src;
    int lineno;
    char msg[100];
} Error;

void py_BaseException__set_lineno(py_Ref, int lineno, const CodeObject* code);
int py_BaseException__get_lineno(py_Ref, const CodeObject* code);
void py_BaseException__stpush(py_Ref, pk_SourceData_ src, int lineno, const char* func_name);

#ifdef __cplusplus
}
#endif