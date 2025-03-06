#pragma once

#include "pocketpy/interpreter/frame.h"
#include "pocketpy/pocketpy.h"

typedef struct Generator{
    py_Frame* frame;
    int state;
} Generator;

void pk_newgenerator(py_Ref out, py_Frame* frame, py_TValue* begin, py_TValue* end);

void Generator__dtor(Generator* ud);