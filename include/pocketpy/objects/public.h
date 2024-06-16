#pragma once

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int16_t pkpy_Type;
typedef struct PyObject PyObject;
typedef struct PyVar PyVar;
typedef struct pkpy_VM pkpy_VM;

struct pkpy_G {
    pkpy_VM* vm;
} extern pkpy_g;

bool py_eq(const PyVar*, const PyVar*);
bool py_le(const PyVar*, const PyVar*);
int64_t py_hash(const PyVar*);

#ifdef __cplusplus
}
#endif
