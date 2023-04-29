#ifndef POCKETPY_C_H 
#define POCKETPY_C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef struct pkpy_vm_handle* pkpy_vm;

pkpy_vm pkpy_vm_create(bool use_stdio, bool enable_os);
void pkpy_vm_exec(pkpy_vm vm_handle, const char* source);
void pkpy_vm_destroy(pkpy_vm vm);

#ifdef __cplusplus
}
#endif

#endif
