#pragma once

#include "pocketpy/common/str.h"
#include "pocketpy/common/strname.h"
#include "pocketpy/objects/codeobject.h"
#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/pocketpy.h"

typedef struct{
    SourceData_ src;
    int lineno;
    char msg[100];
} Error;

void py_BaseException__stpush(py_Ref, SourceData_ src, int lineno, const char* func_name);
