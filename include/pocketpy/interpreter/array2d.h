#pragma once

#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/vm.h"

typedef struct c11_array2d {
    py_TValue* data;  // slots
    int n_cols;
    int n_rows;
    int numel;
} c11_array2d;

typedef struct c11_array2d_iterator {
    c11_array2d* array;
    int index;
} c11_array2d_iterator;

c11_array2d* py_newarray2d(py_OutRef out, int n_cols, int n_rows);
