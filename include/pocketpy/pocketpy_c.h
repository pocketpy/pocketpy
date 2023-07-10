#ifndef POCKETPY_C_H 
#define POCKETPY_C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "export.h"

typedef struct pkpy_vm_handle pkpy_vm;
typedef int (*pkpy_function)(pkpy_vm*); 

/* Basic Functions */
PK_EXPORT pkpy_vm* pkpy_new_vm(bool enable_os);
PK_EXPORT void pkpy_delete_vm(pkpy_vm* vm);
PK_EXPORT bool pkpy_vm_exec(pkpy_vm* vm, const char* source);
PK_EXPORT bool pkpy_vm_exec_2(pkpy_vm* vm, const char* source, const char* filename, int mode, const char* module);

/* Stack Manipulation */
PK_EXPORT bool pkpy_pop(pkpy_vm*, int n);
PK_EXPORT bool pkpy_dup_top(pkpy_vm*);
PK_EXPORT bool pkpy_rot_two(pkpy_vm*);
PK_EXPORT bool pkpy_push_function(pkpy_vm*, pkpy_function, int);
PK_EXPORT bool pkpy_push_int(pkpy_vm*, int);
PK_EXPORT bool pkpy_push_float(pkpy_vm*, double);
PK_EXPORT bool pkpy_push_bool(pkpy_vm*, bool);
PK_EXPORT bool pkpy_push_string(pkpy_vm*, const char*);
PK_EXPORT bool pkpy_push_stringn(pkpy_vm*, const char*, int length);
PK_EXPORT bool pkpy_push_voidp(pkpy_vm*, void*);
PK_EXPORT bool pkpy_push_none(pkpy_vm*);
PK_EXPORT bool pkpy_push_eval(pkpy_vm*, const char* source);
PK_EXPORT bool pkpy_push_module(pkpy_vm*, const char* name);

/* Error Handling */

PK_EXPORT bool pkpy_clear_error(pkpy_vm*, char** message);
PK_EXPORT bool pkpy_error(pkpy_vm*, const char* name, const char* message);
//will return true if the vm is currently in an error state
PK_EXPORT bool pkpy_check_error(pkpy_vm*);

/* Variables */

PK_EXPORT bool pkpy_set_global(pkpy_vm*, const char* name);
PK_EXPORT bool pkpy_get_global(pkpy_vm*, const char* name);
//will return true if global exists
PK_EXPORT bool pkpy_check_global(pkpy_vm*, const char* name);
PK_EXPORT bool pkpy_getattr(pkpy_vm*, const char* name);
PK_EXPORT bool pkpy_setattr(pkpy_vm*, const char* name);

/* Callables */

PK_EXPORT bool pkpy_call(pkpy_vm*, int argc);
PK_EXPORT bool pkpy_call_method(pkpy_vm*, const char* name, int argc);

/* Types */

PK_EXPORT bool pkpy_to_int(pkpy_vm*, int index, int* ret);
PK_EXPORT bool pkpy_to_float(pkpy_vm*, int index, double* ret);
PK_EXPORT bool pkpy_to_bool(pkpy_vm*, int index, bool* ret);
PK_EXPORT bool pkpy_to_voidp(pkpy_vm*, int index, void** ret);

//this method provides a strong reference, you are responsible for freeing the
//string when you are done with it
PK_EXPORT bool pkpy_to_string(pkpy_vm*, int index, char** ret);

//this method provides a weak reference, it is only valid until the
//next api call
//it is not null terminated
PK_EXPORT bool pkpy_to_stringn(pkpy_vm*, int index, const char** ret, int* size);

//these do not follow the same error semantics as above, their return values
//just say whether the check succeeded or not, or else return the value asked for

PK_EXPORT bool pkpy_is_int(pkpy_vm*, int index);
PK_EXPORT bool pkpy_is_float(pkpy_vm*, int index);
PK_EXPORT bool pkpy_is_bool(pkpy_vm*, int index);
PK_EXPORT bool pkpy_is_string(pkpy_vm*, int index);
PK_EXPORT bool pkpy_is_voidp(pkpy_vm*, int index);
PK_EXPORT bool pkpy_is_none(pkpy_vm*, int index);

/* special api */

// free a pointer allocated from pkpy's heap
PK_EXPORT void pkpy_free(void* p);
PK_EXPORT void pkpy_vm_compile(void* vm, const char* source, const char* filename, int mode, bool* ok, char** res);
PK_EXPORT void* pkpy_new_repl(void* vm);
PK_EXPORT bool pkpy_repl_input(void* r, const char* line);
PK_EXPORT void pkpy_delete_repl(void* repl);

#ifdef __cplusplus
}
#endif


#endif
