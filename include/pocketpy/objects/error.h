#pragma once

#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/pocketpy.h"

typedef struct{
    SourceData_ src;
    int lineno;
    char msg[512];
} Error;

void py_BaseException__stpush(py_Frame* frame, py_Ref, SourceData_ src, int lineno, const char* func_name);
