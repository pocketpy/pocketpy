#include "pocketpy.h"
#include "pocketpy_c.h"

using namespace pkpy;

#define SAFEGUARD_OPEN try { \

#define SAFEGUARD_CLOSE \
    } catch(std::exception& e) { \
        std::cerr << "ERROR: a std::exception " \
        << "this probably means pocketpy itself has a bug!\n" \
        << e.what() << "\n"; \
        exit(2); \
    } catch(...) { \
        std::cerr << "ERROR: a unknown exception was thrown " \
        << "this probably means pocketpy itself has a bug!\n"; \
        exit(2); \
    }


#define ERRHANDLER_OPEN SAFEGUARD_OPEN \
    try { \
    if (vm->c_data.top() == nullptr) \
        return false; \

#define ERRHANDLER_CLOSE \
    } catch( Exception e ) { \
        vm->c_data.push(VAR(e)); \
        vm->c_data.push(NULL); \
        return false; \
    } \
    SAFEGUARD_CLOSE \


//for now I will unpack a tuple automatically, we may not want to handle
//it this way, not sure
//it is more lua like, but maybe not python like
static void unpack_return(VM* vm, PyObject* ret) {
    if (is_type(ret, vm->tp_tuple)) {
        Tuple& t = CAST(Tuple&, ret);
        for (int i = 0; i < t.size(); i++) 
            vm->c_data.push(t[i]);
    } else if (ret == vm->None) {
        //do nothing here
        //having to pop the stack after every call that returns none is annoying
        //lua does not do this
        //
        //so for now we will not push none on the stack when it is the sole thing returned
        //if this becomes a problem we can change it
        //
        //you can still check if it returned none by comparing stack size before
        //and after if you have too
    } else 
        vm->c_data.push(ret);

}


bool pkpy_clear_error(pkpy_vm vm_handle, char** message) {
    VM* vm = (VM*) vm_handle;
    SAFEGUARD_OPEN

        if (vm->c_data.top() != nullptr) 
            return false;

        vm->c_data.pop();
        Exception& e = CAST(Exception&, vm->c_data.top());
        if (message != nullptr) 
            *message = e.summary().c_str_dup();
        else
            std::cerr << "ERROR: " << e.summary() << "\n";

        vm->c_data.clear();
        vm->callstack.clear();
        vm->s_data.clear(); 
        return true;

    SAFEGUARD_CLOSE
}

pkpy_vm pkpy_vm_create(bool use_stdio, bool enable_os) {
    VM* vm = new VM(use_stdio, enable_os);

    return (pkpy_vm) vm;
}

bool pkpy_vm_run(pkpy_vm vm_handle, const char* source) {
    VM* vm = (VM*) vm_handle;
    ERRHANDLER_OPEN

    CodeObject_ code = vm->compile(source, "<c-bound>", EXEC_MODE);
    PyObject* result = vm->_exec(code, vm->_main);
    unpack_return(vm, result);

    return true;
    ERRHANDLER_CLOSE
}

void pkpy_vm_destroy(pkpy_vm vm_handle) {
    VM* vm = (VM*) vm_handle;
    delete vm;
}

static void propagate_if_errored(VM* vm) {
    try {
        if (vm->c_data.top() != nullptr) 
            return;

        vm->c_data.pop();
        Exception& e = CAST(Exception&, vm->c_data.top());
        vm->c_data.pop();

        throw e;
    } catch(Exception& e) {
        throw;
    } catch(...) {
        std::cerr << "ERROR: a non pocketpy exeception was thrown " 
            << "this probably means pocketpy itself has a bug!\n"; 
        exit(2); 
    }
}


PyObject* c_function_wrapper(VM* vm, ArgsView args) {
    LuaStyleFuncC f = CAST(NativeFunc&, args[-2])._lua_f;

    //setup c stack
    int stored = vm->c_data.store();

    for (int i = 0; i < args.size(); i++)
        vm->c_data.push(args[i]);
    
    int retc = f(vm);

    PyObject* ret = vm->None;
    propagate_if_errored(vm);

    if (retc == 1) 
        ret = vm->c_data.top();
    else if (retc > 1) {
        Tuple t = Tuple(retc);

        for (int i = 0; i < retc; i++)  {
            int stack_index = (vm->c_data.size() - retc) + i;
            t[i] = vm->c_data.get(stack_index);
        }

        ret = VAR(t);
    }

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

bool pkpy_push_bool(pkpy_vm vm_handle, bool value) {
    VM* vm = (VM*) vm_handle;

    ERRHANDLER_OPEN
    vm->c_data.push(VAR(value));

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_push_string(pkpy_vm vm_handle, const char* value) {
    VM* vm = (VM*) vm_handle;

    ERRHANDLER_OPEN
    vm->c_data.push(VAR(value));

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_push_stringn(pkpy_vm vm_handle, const char* value, int length) {
    VM* vm = (VM*) vm_handle;

    ERRHANDLER_OPEN

    Str s = Str(value, length);
    vm->c_data.push(VAR(s));

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_push_none(pkpy_vm vm_handle) {
    VM* vm = (VM*) vm_handle;

    ERRHANDLER_OPEN
    vm->c_data.push(vm->None);

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

//get global will also get bulitins
bool pkpy_get_global(pkpy_vm vm_handle, const char* name) {
    VM* vm = (VM*) vm_handle;

    ERRHANDLER_OPEN

    PyObject* o = vm->_main->attr().try_get(name);
    if (o == nullptr) {
        o = vm->builtins->attr().try_get(name);
        if (o == nullptr)
            throw Exception("AttributeError", "could not find requested global");
    }

    vm->c_data.push(o);

    return true;
    ERRHANDLER_CLOSE
}


bool pkpy_call(pkpy_vm vm_handle, int argc) {
    VM* vm = (VM*) vm_handle;
    ERRHANDLER_OPEN

    int callable_index = vm->c_data.size() - argc  - 1;

    PyObject* callable = vm->c_data.get(callable_index);

    vm->s_data.push(callable);
    vm->s_data.push(PY_NULL);

    for (int i = 0; i < argc; i++) 
        vm->s_data.push(vm->c_data.get(callable_index + i + 1));

    PyObject* o = vm->vectorcall(argc);

    vm->c_data.shrink(argc + 1);

    unpack_return(vm, o);

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_call_method(pkpy_vm vm_handle, const char* name, int argc) {
    VM* vm = (VM*) vm_handle;
    ERRHANDLER_OPEN

    int self_index = vm->c_data.size() - argc  - 1;
    PyObject* self = vm->c_data.get(self_index);

    PyObject* callable = vm->get_unbound_method(self, name, &self);

    vm->s_data.push(callable);
    vm->s_data.push(self);

    for (int i = 0; i < argc; i++) 
        vm->s_data.push(vm->c_data.get(self_index + i + 1));

    PyObject* o = vm->vectorcall(argc);

    vm->c_data.shrink(argc + 1);

    unpack_return(vm, o);

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

bool pkpy_to_float(pkpy_vm vm_handle, int index, double* ret) {
    VM* vm = (VM*) vm_handle;
    ERRHANDLER_OPEN

    index = lua_to_cstack_index(index, vm->c_data.size());

    PyObject* o = vm->c_data.get(index);
    if (ret != nullptr)
        *ret = py_cast<double>(vm, o);

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_to_bool(pkpy_vm vm_handle, int index, bool* ret) {
    VM* vm = (VM*) vm_handle;
    ERRHANDLER_OPEN

    index = lua_to_cstack_index(index, vm->c_data.size());

    PyObject* o = vm->c_data.get(index);
    if (ret != nullptr)
        *ret = py_cast<bool>(vm, o);

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_to_string(pkpy_vm vm_handle, int index, char** ret) {
    VM* vm = (VM*) vm_handle;
    ERRHANDLER_OPEN

    index = lua_to_cstack_index(index, vm->c_data.size());

    PyObject* o = vm->c_data.get(index);
    if (ret != nullptr) {
        Str& s = CAST(Str&, o);
        *ret = s.c_str_dup();
    }

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_is_int(pkpy_vm vm_handle, int index) {
    VM* vm = (VM*) vm_handle;

    index = lua_to_cstack_index(index, vm->c_data.size());
    PyObject* o = vm->c_data.get(index);

    return is_type(o, vm->tp_int);
}
bool pkpy_is_float(pkpy_vm vm_handle, int index) {
    VM* vm = (VM*) vm_handle;

    index = lua_to_cstack_index(index, vm->c_data.size());
    PyObject* o = vm->c_data.get(index);

    return is_type(o, vm->tp_float);
}
bool pkpy_is_bool(pkpy_vm vm_handle, int index) {
    VM* vm = (VM*) vm_handle;

    index = lua_to_cstack_index(index, vm->c_data.size());
    PyObject* o = vm->c_data.get(index);

    return is_type(o, vm->tp_bool);
}
bool pkpy_is_string(pkpy_vm vm_handle, int index) {
    VM* vm = (VM*) vm_handle;

    index = lua_to_cstack_index(index, vm->c_data.size());
    PyObject* o = vm->c_data.get(index);

    return is_type(o, vm->tp_str);
}
bool pkpy_is_none(pkpy_vm vm_handle, int index) {
    VM* vm = (VM*) vm_handle;

    index = lua_to_cstack_index(index, vm->c_data.size());
    PyObject* o = vm->c_data.get(index);

    return o == vm->None;
}

bool pkpy_check_stack(pkpy_vm vm_handle, int free) {
    VM* vm = (VM*) vm_handle;
    return free <= vm->c_data.remaining();
}

int pkpy_stack_size(pkpy_vm vm_handle) {
    VM* vm = (VM*) vm_handle;
    return vm->c_data.size();
}

