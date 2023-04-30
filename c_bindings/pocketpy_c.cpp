#include "pocketpy.h"
#include "pocketpy_c.h"

using namespace pkpy;


#define ERRHANDLER_OPEN try { \
    try { \
    if (vm->c_data.top() == nullptr) \
        return false; \

#define ERRHANDLER_CLOSE \
    } catch( Exception e ) { \
        vm->c_data.clear(); \
        vm->c_data.push(VAR(e.summary())); \
        vm->c_data.push(NULL); \
        return false; \
    } \
    } catch(...) { \
        std::cerr << "ERROR: a non pocketpy exeception was thrown " \
        << "this probably means pocketpy itself has a bug!\n"; \
        exit(2); \
    }

bool pkpy_clear_error(pkpy_vm vm_handle, const char** message) {
    VM* vm = (VM*) vm_handle;

    try {
        if (vm->c_data.top() != nullptr) 
            return false;

        vm->c_data.pop();
        Str wrapped_message = CAST(Str&, vm->c_data.top());
        if (message != nullptr) 
            *message = wrapped_message.c_str_dup();
        else
            std::cerr << "ERROR: " << wrapped_message << "\n";

        vm->c_data.pop();
        //at this point the stack is clear

        return true;

    } catch(...) {
        std::cerr << "ERROR: a non pocketpy exeception was thrown " 
        << "this probably means pocketpy itself has a bug!\n"; 
        exit(2); 
    }
}

pkpy_vm pkpy_vm_create(bool use_stdio, bool enable_os) {
    VM* vm = new VM(use_stdio, enable_os);

    return (pkpy_vm) vm;
}

bool pkpy_vm_exec(pkpy_vm vm_handle, const char* source) {
    VM* vm = (VM*) vm_handle;
    ERRHANDLER_OPEN

    vm->exec(source, "main.py", EXEC_MODE);

    return true;
    ERRHANDLER_CLOSE
}

void pkpy_vm_destroy(pkpy_vm vm_handle) {
    VM* vm = (VM*) vm_handle;
    delete vm;
}

PyObject* c_function_wrapper(VM* vm, ArgsView args) {
    LuaStyleFuncC f = CAST(NativeFunc&, args[-2])._lua_f;

    //setup c stack
    int stored = vm->c_data.store();

    for (int i = 0; i < args.size(); i++)
        vm->c_data.push(args[i]);
    
    int retc = f(vm);

    PyObject* ret = vm->None;

    //TODO handle tuple packing for multiple returns
    if (retc > 0) 
        ret = vm->c_data.top();

    vm->c_data.clear();
    vm->c_data.restore(stored);

    return ret;
}

bool pkpy_push_function(pkpy_vm vm_handle, pkpy_function f) {
    VM* vm = (VM*) vm_handle;
    ERRHANDLER_OPEN

    //TODO right now we just treat all c bound functions a varargs functions
    //do we want to change that?
    NativeFunc nf = NativeFunc(c_function_wrapper, -1, 0);
    nf._lua_f = (LuaStyleFuncC) f;

    vm->c_data.push(VAR(nf));

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_push_int(pkpy_vm vm_handle, int value) {
    VM* vm = (VM*) vm_handle;
    ERRHANDLER_OPEN

    vm->c_data.push(VAR(value));

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_push_float(pkpy_vm vm_handle, double value) {
    VM* vm = (VM*) vm_handle;

    ERRHANDLER_OPEN
    vm->c_data.push(VAR(value));

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_set_global(pkpy_vm vm_handle, const char* name) {
    VM* vm = (VM*) vm_handle;

    ERRHANDLER_OPEN

    vm->_main->attr().set(name, vm->c_data.top());

    vm->c_data.pop();

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_get_global(pkpy_vm vm_handle, const char* name) {
    VM* vm = (VM*) vm_handle;

    ERRHANDLER_OPEN

    PyObject* o = vm->_main->attr().try_get(name);

    vm->c_data.push(o);

    return true;
    ERRHANDLER_CLOSE
}

static void call_wrapper(VM* vm, int argc, bool method_call) {
    int callable_index = vm->c_data.size() - argc  - 1;

    PyObject* callable = vm->c_data.get(callable_index);

    vm->s_data.push(callable);
    if (method_call) 
        vm->s_data.push(vm->c_data.get(callable_index - 1));
    else 
        vm->s_data.push(PY_NULL);

    for (int i = 0; i < argc; i++) 
        vm->s_data.push(vm->c_data.get(callable_index + i + 1));

    PyObject* o = vm->vectorcall(argc);

    vm->c_data.shrink(argc + 1 + (int) method_call);

    //TODO unpack tuple? 
    vm->c_data.push(o);
}

bool pkpy_call(pkpy_vm vm_handle, int argc) {
    VM* vm = (VM*) vm_handle;
    ERRHANDLER_OPEN
    call_wrapper(vm, argc, false);

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_call_method(pkpy_vm vm_handle, int argc) {
    VM* vm = (VM*) vm_handle;
    ERRHANDLER_OPEN
    call_wrapper(vm, argc, true);

    return true;
    ERRHANDLER_CLOSE
}

static int lua_to_cstack_index(int index, int size) {
    if (index < 0)
        index = size + index;
    return index;
}

bool pkpy_to_int(pkpy_vm vm_handle, int index, int* ret) {
    VM* vm = (VM*) vm_handle;
    ERRHANDLER_OPEN

    index = lua_to_cstack_index(index, vm->c_data.size());

    PyObject* o = vm->c_data.get(index);
    if (ret != nullptr)
        *ret = py_cast<int>(vm, o);

    return true;
    ERRHANDLER_CLOSE
}
