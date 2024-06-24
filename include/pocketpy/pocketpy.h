#include "stdint.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PyObject PyObject;
typedef struct PyVar PyVar;
typedef struct pk_VM pk_VM;
typedef struct py_Error py_Error;

typedef int (*py_CFunction)(const PyVar*, int);

extern pk_VM* pk_current_vm;

void py_initialize();
// void py_switch_vm(const char* name);
void py_finalize();

py_Error* py_exec_simple(const char*);
py_Error* py_eval_simple(const char*, PyVar*);

/* py_error */
void py_Error__print(const py_Error*);
void py_Error__delete(py_Error*);


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

#define py_isnull(self) ((self)->type == 0)

#ifdef __cplusplus
}
#endif
