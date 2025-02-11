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

/* chunked_array2d */
#define SMALLMAP_T__HEADER
#define K c11_vec2i
#define V py_TValue*
#define NAME c11_chunked_array2d_chunks
#define less(a, b) (a._i64 < b._i64)
#define equal(a, b) (a._i64 == b._i64)
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__HEADER

typedef struct c11_chunked_array2d {
    c11_chunked_array2d_chunks chunks;
    int chunk_size;
    int chunk_size_log2;
    int chunk_size_mask;
    c11_chunked_array2d_chunks_KV last_visited;

    py_TValue default_T;
    py_TValue context_builder;
} c11_chunked_array2d;

void c11_chunked_array2d__dtor(c11_chunked_array2d* self);

py_Ref c11_chunked_array2d__get(c11_chunked_array2d* self, int col, int row);
bool c11_chunked_array2d__set(c11_chunked_array2d* self, int col, int row, py_Ref value) PY_RAISE;
void c11_chunked_array2d__del(c11_chunked_array2d* self, int col, int row);

void pk__register_chunked_array2d(py_Ref mod);

/* array2d_view */