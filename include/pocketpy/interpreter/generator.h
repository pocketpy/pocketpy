#pragma once

#include "pocketpy/interpreter/frame.h"

typedef struct Generator{
    Frame* frame;
    int state;
} Generator;

void pk_newgenerator(py_Ref out, Frame* frame, int slots);