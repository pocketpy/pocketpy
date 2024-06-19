#pragma once

#include "stdint.h"
#include "stdbool.h"

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

void py_initialize();
void py_switch_vm(const char* name);
void py_finalize();

bool py_eq(const PyVar*, const PyVar*);
bool py_le(const PyVar*, const PyVar*);
int64_t py_hash(const PyVar*);


/* py_var */
void py_newint(PyVar*, int64_t);
void py_newfloat(PyVar*, double);
void py_newbool(PyVar*, bool);
void py_newstr(PyVar*, const char*);
void py_newstr2(PyVar*, const char*, int);
void py_newbytes(PyVar*, const uint8_t*, int);
void py_newnone(PyVar*);

#ifdef __cplusplus
}
#endif
