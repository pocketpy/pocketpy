#pragma once

#include "pocketpy/pocketpy.h"
#include "pocketpy/common/smallmap.h"
#include "pocketpy/objects/base.h"

typedef struct c11_array2d_like {
    int n_cols;
    int n_rows;
    int numel;
    py_Ref (*f_get)(struct c11_array2d_like* self, int col, int row);
    bool (*f_set)(struct c11_array2d_like* self, int col, int row, py_Ref value);
} c11_array2d_like;

typedef struct c11_array2d_like_iterator {
    c11_array2d_like* array;
    int j;
    int i;
} c11_array2d_like_iterator;

typedef struct c11_array2d {
    c11_array2d_like header;
    py_TValue* data;  // slots
} c11_array2d;

typedef struct c11_array2d_view {
    c11_array2d_like header;
    void* ctx;
    py_Ref (*f_get)(void* ctx, int col, int row);
    bool (*f_set)(void* ctx, int col, int row, py_Ref value);
    c11_vec2i origin;
} c11_array2d_view;

c11_array2d* c11_newarray2d(py_OutRef out, int n_cols, int n_rows);

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

py_Ref c11_chunked_array2d__get(c11_chunked_array2d* self, int col, int row);
bool c11_chunked_array2d__set(c11_chunked_array2d* self, int col, int row, py_Ref value);
