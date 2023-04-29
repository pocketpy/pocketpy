#ifndef POCKETPY_C_H 
#define POCKETPY_C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef struct pkpy_vm_handle* pkpy_vm;

pkpy_vm pkpy_vm_create(bool use_stdio, bool enable_os);
void pkpy_vm_exec(pkpy_vm vm_handle, const char* source);
void pkpy_vm_destroy(pkpy_vm vm);

////////binding a c function to pocketpy
typedef void (*pkpy_cfunction)(pkpy_vm); 

void pkpy_push_cfunction(pkpy_vm, pkpy_cfunction);
void pkpy_push_int(pkpy_vm, int64_t);
void pkpy_push_float(pkpy_vm, double);

void pkpy_set_global(pkpy_vm, const char* name);





#ifdef __cplusplus
}
#endif

#endif
