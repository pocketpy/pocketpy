#ifndef POCKETPY_C_H 
#define POCKETPY_C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "export.h"

typedef struct pkpy_vm_handle pkpy_vm;
typedef int (*pkpy_CFunction)(pkpy_vm*);
typedef void (*pkpy_COutputHandler)(const char*, int);
typedef unsigned char* (*pkpy_CImportHandler)(const char*, int, int*);
typedef int pkpy_CName;
typedef int pkpy_CType;
typedef const char* pkpy_CString;

/* Basic Functions */
PK_EXPORT pkpy_vm* pkpy_new_vm(bool enable_os);
PK_EXPORT void pkpy_delete_vm(pkpy_vm*);
PK_EXPORT bool pkpy_exec(pkpy_vm*, const char* source);
PK_EXPORT bool pkpy_exec_2(pkpy_vm*, const char* source, const char* filename, int mode, const char* module);
PK_EXPORT void pkpy_set_main_argv(pkpy_vm*, int argc, char** argv);

/* Stack Manipulation */
PK_EXPORT bool pkpy_dup(pkpy_vm*, int i);
PK_EXPORT bool pkpy_pop(pkpy_vm*, int n);
PK_EXPORT bool pkpy_pop_top(pkpy_vm*);
PK_EXPORT bool pkpy_dup_top(pkpy_vm*);
PK_EXPORT bool pkpy_rot_two(pkpy_vm*);
PK_EXPORT int pkpy_stack_size(pkpy_vm*);

// int
PK_EXPORT bool pkpy_push_int(pkpy_vm*, int val);
PK_EXPORT bool pkpy_is_int(pkpy_vm*, int i);
PK_EXPORT bool pkpy_to_int(pkpy_vm*, int i, int* out);

// float
PK_EXPORT bool pkpy_push_float(pkpy_vm*, double val);
PK_EXPORT bool pkpy_is_float(pkpy_vm*, int i);
PK_EXPORT bool pkpy_to_float(pkpy_vm*, int i, double* out);

// bool
PK_EXPORT bool pkpy_push_bool(pkpy_vm*, bool val);
PK_EXPORT bool pkpy_is_bool(pkpy_vm*, int i);
PK_EXPORT bool pkpy_to_bool(pkpy_vm*, int i, bool* out);

// string
PK_EXPORT bool pkpy_push_string(pkpy_vm*, pkpy_CString val);
PK_EXPORT bool pkpy_is_string(pkpy_vm*, int i);
PK_EXPORT bool pkpy_to_string(pkpy_vm*, int i, pkpy_CString* out);

// void_p
PK_EXPORT bool pkpy_push_voidp(pkpy_vm*, void* val);
PK_EXPORT bool pkpy_is_voidp(pkpy_vm*, int i);
PK_EXPORT bool pkpy_to_voidp(pkpy_vm*, int i, void** out);

// none
PK_EXPORT bool pkpy_push_none(pkpy_vm*);
PK_EXPORT bool pkpy_is_none(pkpy_vm*, int i);

// special push
PK_EXPORT bool pkpy_push_null(pkpy_vm*);
PK_EXPORT bool pkpy_push_function(pkpy_vm*, const char* sig, pkpy_CFunction val);
PK_EXPORT bool pkpy_push_module(pkpy_vm*, const char* name);

// some opt
PK_EXPORT bool pkpy_getattr(pkpy_vm*, pkpy_CName name);
PK_EXPORT bool pkpy_setattr(pkpy_vm*, pkpy_CName name);
PK_EXPORT bool pkpy_getglobal(pkpy_vm*, pkpy_CName name);
PK_EXPORT bool pkpy_setglobal(pkpy_vm*, pkpy_CName name);
PK_EXPORT bool pkpy_eval(pkpy_vm*, const char* source);
PK_EXPORT bool pkpy_unpack_sequence(pkpy_vm*, int size);
PK_EXPORT bool pkpy_get_unbound_method(pkpy_vm*, pkpy_CName name);
PK_EXPORT bool pkpy_py_repr(pkpy_vm*);
PK_EXPORT bool pkpy_py_str(pkpy_vm*);

/* Error Handling */
PK_EXPORT bool pkpy_error(pkpy_vm*, const char* name, pkpy_CString msg);
PK_EXPORT bool pkpy_check_error(pkpy_vm*);
PK_EXPORT bool pkpy_clear_error(pkpy_vm*, char** message);

/* Callables */
PK_EXPORT bool pkpy_vectorcall(pkpy_vm*, int argc);

/* Special APIs */
PK_EXPORT void pkpy_free(void* p);
#define pkpy_string(__s) (__s)
PK_EXPORT pkpy_CName pkpy_name(const char* s);
PK_EXPORT pkpy_CString pkpy_name_to_string(pkpy_CName name);
PK_EXPORT void pkpy_set_output_handler(pkpy_vm*, pkpy_COutputHandler handler);
PK_EXPORT void pkpy_set_import_handler(pkpy_vm*, pkpy_CImportHandler handler);

/* REPL */
PK_EXPORT void* pkpy_new_repl(pkpy_vm*);
PK_EXPORT bool pkpy_repl_input(void* r, const char* line);
PK_EXPORT void pkpy_delete_repl(void* repl);
#ifdef __cplusplus
}
#endif


#endif
