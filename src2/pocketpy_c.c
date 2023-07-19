
#include "pocketpy_c.h"

#ifdef _WIN32
#pragma warning(disable: 4700)
#endif
            
pkpy_vm* pkpy_new_vm(bool enable_os) {
    pkpy_vm* returnValue;
    return returnValue;
}

void pkpy_delete_vm(pkpy_vm*) {

}

bool pkpy_exec(pkpy_vm*, const char* source) {
    bool returnValue;
    return returnValue;
}

bool pkpy_exec_2(pkpy_vm*, const char* source, const char* filename, int mode, const char* module) {
    bool returnValue;
    return returnValue;
}

bool pkpy_dup(pkpy_vm*, int) {
    bool returnValue;
    return returnValue;
}

bool pkpy_pop(pkpy_vm*, int) {
    bool returnValue;
    return returnValue;
}

bool pkpy_pop_top(pkpy_vm*) {
    bool returnValue;
    return returnValue;
}

bool pkpy_dup_top(pkpy_vm*) {
    bool returnValue;
    return returnValue;
}

bool pkpy_rot_two(pkpy_vm*) {
    bool returnValue;
    return returnValue;
}

int pkpy_stack_size(pkpy_vm*) {
    int returnValue;
    return returnValue;
}

bool pkpy_push_int(pkpy_vm*, int) {
    bool returnValue;
    return returnValue;
}

bool pkpy_is_int(pkpy_vm*, int i) {
    bool returnValue;
    return returnValue;
}

bool pkpy_to_int(pkpy_vm*, int i, int* out) {
    bool returnValue;
    return returnValue;
}

bool pkpy_push_float(pkpy_vm*, float) {
    bool returnValue;
    return returnValue;
}

bool pkpy_is_float(pkpy_vm*, int i) {
    bool returnValue;
    return returnValue;
}

bool pkpy_to_float(pkpy_vm*, int i, float* out) {
    bool returnValue;
    return returnValue;
}

bool pkpy_push_bool(pkpy_vm*, bool) {
    bool returnValue;
    return returnValue;
}

bool pkpy_is_bool(pkpy_vm*, int i) {
    bool returnValue;
    return returnValue;
}

bool pkpy_to_bool(pkpy_vm*, int i, bool* out) {
    bool returnValue;
    return returnValue;
}

bool pkpy_push_string(pkpy_vm*, pkpy_CString) {
    bool returnValue;
    return returnValue;
}

bool pkpy_is_string(pkpy_vm*, int i) {
    bool returnValue;
    return returnValue;
}

bool pkpy_to_string(pkpy_vm*, int i, pkpy_CString* out) {
    bool returnValue;
    return returnValue;
}

bool pkpy_push_voidp(pkpy_vm*, void*) {
    bool returnValue;
    return returnValue;
}

bool pkpy_is_voidp(pkpy_vm*, int i) {
    bool returnValue;
    return returnValue;
}

bool pkpy_to_voidp(pkpy_vm*, int i, void** out) {
    bool returnValue;
    return returnValue;
}

bool pkpy_push_none(pkpy_vm*) {
    bool returnValue;
    return returnValue;
}

bool pkpy_is_none(pkpy_vm*, int i) {
    bool returnValue;
    return returnValue;
}

bool pkpy_push_null(pkpy_vm*) {
    bool returnValue;
    return returnValue;
}

bool pkpy_push_function(pkpy_vm*, const char*, pkpy_CFunction) {
    bool returnValue;
    return returnValue;
}

bool pkpy_push_module(pkpy_vm*, const char*) {
    bool returnValue;
    return returnValue;
}

bool pkpy_getattr(pkpy_vm*, pkpy_CName) {
    bool returnValue;
    return returnValue;
}

bool pkpy_setattr(pkpy_vm*, pkpy_CName) {
    bool returnValue;
    return returnValue;
}

bool pkpy_getglobal(pkpy_vm*, pkpy_CName) {
    bool returnValue;
    return returnValue;
}

bool pkpy_setglobal(pkpy_vm*, pkpy_CName) {
    bool returnValue;
    return returnValue;
}

bool pkpy_eval(pkpy_vm*, const char* source) {
    bool returnValue;
    return returnValue;
}

bool pkpy_unpack_sequence(pkpy_vm*, int size) {
    bool returnValue;
    return returnValue;
}

bool pkpy_get_unbound_method(pkpy_vm*, pkpy_CName) {
    bool returnValue;
    return returnValue;
}

bool pkpy_py_repr(pkpy_vm*) {
    bool returnValue;
    return returnValue;
}

bool pkpy_error(pkpy_vm*, const char* name, pkpy_CString) {
    bool returnValue;
    return returnValue;
}

bool pkpy_check_error(pkpy_vm*) {
    bool returnValue;
    return returnValue;
}

bool pkpy_clear_error(pkpy_vm*, char** message) {
    bool returnValue;
    return returnValue;
}

bool pkpy_vectorcall(pkpy_vm*, int argc) {
    bool returnValue;
    return returnValue;
}

void pkpy_free(void* p) {

}

pkpy_CString pkpy_string(const char*) {
    pkpy_CString returnValue;
    return returnValue;
}

pkpy_CName pkpy_name(const char*) {
    pkpy_CName returnValue;
    return returnValue;
}

pkpy_CString pkpy_name_to_string(pkpy_CName) {
    pkpy_CString returnValue;
    return returnValue;
}

void pkpy_compile_to_string(pkpy_vm*, const char* source, const char* filename, int mode, bool* ok, char** out) {

}

void* pkpy_new_repl(pkpy_vm* vm) {
    void* returnValue;
    return returnValue;
}

bool pkpy_repl_input(void* r, const char* line) {
    bool returnValue;
    return returnValue;
}

void pkpy_delete_repl(void* repl) {

}
