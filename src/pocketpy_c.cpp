#include "pocketpy.h"
#include "pocketpy/tuplelist.h"
#include "pocketpy_c.h"

using namespace pkpy;

typedef int (*LuaStyleFuncC)(VM*);

#define ERRHANDLER_OPEN \
    if (vm->_c.error != nullptr) \
        return false; \
    try {

#define ERRHANDLER_CLOSE \
    } catch(Exception& e ) { \
        vm->_c.error = py_var(vm, e); \
        return false; \
    } catch(const std::exception& re){ \
        auto e = Exception("std::exception", re.what()); \
        vm->_c.error = py_var(vm, e); \
        return false; \
    }

pkpy_vm* pkpy_new_vm(bool enable_os){
    return (pkpy_vm*)new VM(enable_os);
}

void pkpy_delete_vm(pkpy_vm* vm){
    return delete (VM*)vm;
}

bool pkpy_vm_exec(pkpy_vm* vm_handle, const char* source) {
    VM* vm = (VM*) vm_handle;
    PyObject* res;
    ERRHANDLER_OPEN
    CodeObject_ code = vm->compile(source, "main.py", EXEC_MODE);
    res = vm->_exec(code, vm->_main);
    ERRHANDLER_CLOSE
    return res != nullptr;
}

bool pkpy_vm_exec_2(pkpy_vm* vm_handle, const char* source, const char* filename, int mode, const char* module){
    VM* vm = (VM*) vm_handle;
    PyObject* res;
    PyObject* mod;
    ERRHANDLER_OPEN
    if(module == nullptr){
        mod = vm->_main;
    }else{
        mod = vm->_modules[module];     // may raise
    }
    CodeObject_ code = vm->compile(source, filename, (CompileMode)mode);
    res = vm->_exec(code, mod);
    ERRHANDLER_CLOSE
    return res != nullptr;
}



//for now I will unpack a tuple automatically, we may not want to handle
//it this way, not sure
//it is more lua like, but maybe not python like
static void unpack_return(CVM* vm, PyObject* ret) {
    if (is_type(ret, vm->tp_tuple)) {
        Tuple& t = _py_cast<Tuple&>(vm, ret);
        for (int i = 0; i < t.size(); i++) 
            vm->c_data->push(t[i]);
    } else if (ret == vm->None) {
        //do nothing here
        //having to pop the stack after every call that returns none is annoying
        //lua does not do this
        //
        //so for now we will not push none on the stack when it is the sole thing returned
        //if this becomes a problem we can change it
        //
        //you can still check if it returned none by comparing stack size before
        //and after if you have to
    } else 
        vm->c_data->push(ret);

}


bool pkpy_clear_error(pkpy_vm* vm_handle, char** message) {
    CVM* vm = (CVM*) vm_handle;
    // no error
    if (vm->error == nullptr) return false;
    Exception& e = _py_cast<Exception&>(vm, vm->error);
    if (message != nullptr) 
        *message = e.summary().c_str_dup();
    else
        std::cerr << "ERROR: " << e.summary() << "\n";
    vm->error = nullptr;
    vm->c_data->clear();
    vm->callstack.clear();
    vm->s_data.clear(); 
    return true;
}

PyObject* c_function_wrapper(VM* vm, ArgsView args) {
    LuaStyleFuncC f = lambda_get_userdata<LuaStyleFuncC>(args.begin());
    PyObject** curr_sp = &vm->s_data.top();
    int retc = f(vm);
    // propagate_if_errored
    if (vm->_c.error != nullptr){
        Exception e = _py_cast<Exception&>(vm, vm->_c.error);
        vm->_c.error = nullptr;
        vm->_error(e);
    }
    PK_ASSERT(retc == vm->s_data._sp-curr_sp);
    if(retc == 0) return vm->None;
    if (retc == 1) return vm->s_data.popx();
    ArgsView ret_view(curr_sp, vm->s_data._sp);
    return py_var(vm, ret_view.to_tuple());
}

bool pkpy_push_function(pkpy_vm* vm_handle, const char* sig, pkpy_function f) {
    VM* vm = (VM*) vm_handle;
    ERRHANDLER_OPEN
    PyObject* f_obj = vm->bind(
        nullptr,
        sig,
        nullptr,
        c_function_wrapper,
        f
    );
    vm->s_data.push(f_obj);
    ERRHANDLER_CLOSE
    return true;
}

bool pkpy_push_int(pkpy_vm* vm_handle, int value) {
    CVM* vm = (CVM*) vm_handle;
    ERRHANDLER_OPEN
    vm->c_data->safe_push(py_var(vm, value));
    ERRHANDLER_CLOSE
    return true;
}

bool pkpy_push_float(pkpy_vm* vm_handle, double value) {
    CVM* vm = (CVM*) vm_handle;
    ERRHANDLER_OPEN
    vm->c_data->safe_push(py_var(vm, value));
    ERRHANDLER_CLOSE
    return true;
}

bool pkpy_push_bool(pkpy_vm* vm_handle, bool value) {
    CVM* vm = (CVM*) vm_handle;
    ERRHANDLER_OPEN
    vm->c_data->safe_push(py_var(vm, value));
    ERRHANDLER_CLOSE
    return true;
}

bool pkpy_push_string(pkpy_vm* vm_handle, const char* value) {
    CVM* vm = (CVM*) vm_handle;
    ERRHANDLER_OPEN
    vm->c_data->safe_push(py_var(vm, value));
    ERRHANDLER_CLOSE
    return true;
}

bool pkpy_push_stringn(pkpy_vm* vm_handle, const char* value, int length) {
    CVM* vm = (CVM*) vm_handle;
    ERRHANDLER_OPEN
    vm->c_data->safe_push(py_var(vm, Str(value, length)));
    ERRHANDLER_CLOSE
    return true;
}

bool pkpy_push_voidp(pkpy_vm* vm_handle, void* value) {
    CVM* vm = (CVM*) vm_handle;
    ERRHANDLER_OPEN
    vm->c_data->safe_push(py_var(vm, value));
    ERRHANDLER_CLOSE
    return true;
}

bool pkpy_push_none(pkpy_vm* vm_handle) {
    CVM* vm = (CVM*) vm_handle;
    ERRHANDLER_OPEN
    vm->c_data->safe_push(vm->None);
    ERRHANDLER_CLOSE
    return true;
}



bool pkpy_set_global(pkpy_vm* vm_handle, const char* name) {
    CVM* vm = (CVM*) vm_handle;
    ERRHANDLER_OPEN
    vm->_main->attr().set(name, vm->c_data->safe_top());
    vm->c_data->safe_pop();
    ERRHANDLER_CLOSE
    return true;
}

//get global will also get bulitins
bool pkpy_get_global(pkpy_vm* vm_handle, const char* name) {
    CVM* vm = (CVM*) vm_handle;
    ERRHANDLER_OPEN
    PyObject* o = vm->_main->attr().try_get(name);
    if (o == nullptr) {
        o = vm->builtins->attr().try_get(name);
        if (o == nullptr)
            throw Exception("NameError", name);
    }
    vm->c_data->safe_push(o);
    ERRHANDLER_CLOSE
    return true;
}


bool pkpy_call(pkpy_vm* vm_handle, int argc) {
    CVM* vm = (CVM*) vm_handle;
    ERRHANDLER_OPEN
    int callable_index = vm->c_data->size() - argc  - 1;
    PyObject* callable = vm->c_data->at(callable_index);
    vm->s_data.push(callable);
    vm->s_data.push(PY_NULL);

    for (int i = 0; i < argc; i++) 
        vm->s_data.push(vm->c_data->at(callable_index + i + 1));

    PyObject* o = vm->vectorcall(argc);

    vm->c_data->shrink(argc + 1);

    unpack_return(vm, o);
    ERRHANDLER_CLOSE

    return true;
}

bool pkpy_call_method(pkpy_vm* vm_handle, const char* name, int argc) {
    CVM* vm = (CVM*) vm_handle;
    ERRHANDLER_OPEN

    int self_index = vm->c_data->size() - argc  - 1;
    PyObject* self = vm->c_data->at(self_index);

    PyObject* callable = vm->get_unbound_method(self, name, &self);

    vm->s_data.push(callable);
    vm->s_data.push(self);

    for (int i = 0; i < argc; i++) 
        vm->s_data.push(vm->c_data->at(self_index + i + 1));

    PyObject* o = vm->vectorcall(argc);
    vm->c_data->shrink(argc + 1);
    unpack_return(vm, o);
    return true;
    ERRHANDLER_CLOSE
}

static int lua_to_cstack_index(int index, int size) {
    if (index < 0) index = size + index;
    return index;
}

bool pkpy_to_int(pkpy_vm* vm_handle, int index, int* ret) {
    CVM* vm = (CVM*) vm_handle;
    ERRHANDLER_OPEN

    index = lua_to_cstack_index(index, vm->c_data->size());

    PyObject* o = vm->c_data->at(index);
    if (ret != nullptr) *ret = py_cast<int>(vm, o);

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_to_float(pkpy_vm* vm_handle, int index, double* ret) {
    CVM* vm = (CVM*) vm_handle;
    ERRHANDLER_OPEN
    index = lua_to_cstack_index(index, vm->c_data->size());
    PyObject* o = vm->c_data->at(index);
    if (ret != nullptr) *ret = py_cast<double>(vm, o);
    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_to_bool(pkpy_vm* vm_handle, int index, bool* ret) {
    CVM* vm = (CVM*) vm_handle;
    ERRHANDLER_OPEN
    index = lua_to_cstack_index(index, vm->c_data->size());
    PyObject* o = vm->c_data->at(index);
    if (ret != nullptr) *ret = py_cast<bool>(vm, o);
    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_to_voidp(pkpy_vm* vm_handle, int index, void** ret) {
    CVM* vm = (CVM*) vm_handle;
    ERRHANDLER_OPEN
    index = lua_to_cstack_index(index, vm->c_data->size());
    PyObject* o = vm->c_data->at(index);
    if (ret != nullptr) *ret = py_cast<void*>(vm, o);
    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_to_string(pkpy_vm* vm_handle, int index, char** ret) {
    CVM* vm = (CVM*) vm_handle;
    ERRHANDLER_OPEN
    index = lua_to_cstack_index(index, vm->c_data->size());
    PyObject* o = vm->c_data->at(index);
    if (ret != nullptr) {
        *ret = py_cast<Str&>(vm, o).c_str_dup();
    }
    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_to_stringn(pkpy_vm* vm_handle, int index, const char** ret, int* size) {
    CVM* vm = (CVM*) vm_handle;
    ERRHANDLER_OPEN
    index = lua_to_cstack_index(index, vm->c_data->size());
    PyObject* o = vm->c_data->at(index);
    if (ret != nullptr) {
        std::string_view sv = py_cast<Str&>(vm, o).sv();
        *ret = sv.data();
        *size = sv.size();
    }
    return true;
    ERRHANDLER_CLOSE
}


bool pkpy_is_int(pkpy_vm* vm_handle, int index) {
    CVM* vm = (CVM*) vm_handle;
    index = lua_to_cstack_index(index, vm->c_data->size());
    PyObject* o = vm->c_data->at(index);
    return is_type(o, vm->tp_int);
}
bool pkpy_is_float(pkpy_vm* vm_handle, int index) {
    CVM* vm = (CVM*) vm_handle;
    index = lua_to_cstack_index(index, vm->c_data->size());
    PyObject* o = vm->c_data->at(index);
    return is_type(o, vm->tp_float);
}
bool pkpy_is_bool(pkpy_vm* vm_handle, int index) {
    CVM* vm = (CVM*) vm_handle;
    index = lua_to_cstack_index(index, vm->c_data->size());
    PyObject* o = vm->c_data->at(index);
    return is_type(o, vm->tp_bool);
}
bool pkpy_is_string(pkpy_vm* vm_handle, int index) {
    CVM* vm = (CVM*) vm_handle;
    index = lua_to_cstack_index(index, vm->c_data->size());
    PyObject* o = vm->c_data->at(index);
    return is_type(o, vm->tp_str);
}
bool pkpy_is_voidp(pkpy_vm* vm_handle, int index) {
    CVM* vm = (CVM*) vm_handle;
    index = lua_to_cstack_index(index, vm->c_data->size());
    PyObject* o = vm->c_data->at(index);
    return is_type(o, VoidP::_type(vm));
}

bool pkpy_is_none(pkpy_vm* vm_handle, int index) {
    CVM* vm = (CVM*) vm_handle;
    index = lua_to_cstack_index(index, vm->c_data->size());
    PyObject* o = vm->c_data->at(index);
    return o == vm->None;
}

bool pkpy_check_global(pkpy_vm* vm_handle, const char* name) {
    CVM* vm = (CVM*) vm_handle;
    PyObject* o = vm->_main->attr().try_get(name);
    if (o == nullptr) {
        o = vm->builtins->attr().try_get(name);
        if (o == nullptr)
            return false;
    }
    return true;
}

bool pkpy_check_error(pkpy_vm* vm_handle) {
    CVM* vm = (CVM*) vm_handle;
    return vm->error != nullptr;
}


bool pkpy_check_stack(pkpy_vm* vm_handle, int free) {
    CVM* vm = (CVM*) vm_handle;
    return free + vm->c_data->size() <= LuaStack::max_size();
}

int pkpy_stack_size(pkpy_vm* vm_handle) {
    CVM* vm = (CVM*) vm_handle;
    return vm->c_data->size();
}

bool pkpy_pop(pkpy_vm* vm_handle, int n) {
    CVM* vm = (CVM*) vm_handle;
    vm->c_data->shrink(n);
    return true;
}


bool pkpy_push(pkpy_vm* vm_handle, int index) {
    CVM* vm = (CVM*) vm_handle;
    index = lua_to_cstack_index(index, vm->c_data->size());
    vm->c_data->safe_push(vm->c_data->at(index));
    return true;
}

bool pkpy_error(pkpy_vm* vm_handle, const char* name, const char* message) {
    CVM* vm = (CVM*) vm_handle;
    // already in error state
    if (vm->error != nullptr) return false;
    vm->error = py_var(vm, Exception(name, message));
    return false;
}

bool pkpy_getattr(pkpy_vm* vm_handle, const char* name) {
    CVM* vm = (CVM*) vm_handle;
    ERRHANDLER_OPEN
    PyObject* o = vm->c_data->safe_top();
    PyObject* ret = vm->getattr(o, name, false);
    if(ret == nullptr) return false;
    vm->c_data->top() = ret;
    ERRHANDLER_CLOSE
    return true;
}

bool pkpy_setattr(pkpy_vm* vm_handle, const char* name) {
    CVM* vm = (CVM*) vm_handle;
    ERRHANDLER_OPEN
    if(vm->c_data->size() < 2){
        throw std::runtime_error("not enough arguments");
    }
    PyObject* a = vm->c_data->top();
    PyObject* val = vm->c_data->second();
    vm->setattr(a, name, val);
    vm->c_data->shrink(2);
    ERRHANDLER_CLOSE
    return true;
}

bool pkpy_eval(pkpy_vm* vm_handle, const char* code) {
    CVM* vm = (CVM*) vm_handle;
    ERRHANDLER_OPEN
    CodeObject_ co = vm->compile(code, "<eval>", EVAL_MODE);
    PyObject* ret = vm->_exec(co, vm->_main);
    vm->c_data->safe_push(ret);
    ERRHANDLER_CLOSE
    return true;
}

bool pkpy_new_module(pkpy_vm* vm_handle, const char* name){
    CVM* vm = (CVM*) vm_handle;
    ERRHANDLER_OPEN
    PyObject* mod = vm->new_module(name);
    vm->c_data->safe_push(mod);
    ERRHANDLER_CLOSE
    return true;
}

/*****************************************************************/
    void pkpy_free(void* p){
        free(p);
    }

    void pkpy_vm_compile(void* vm_, const char* source, const char* filename, int mode, bool* ok, char** res){
        VM* vm = (VM*)vm_;
        try{
            CodeObject_ code = vm->compile(source, filename, (CompileMode)mode);
            *res = code->serialize(vm).c_str_dup();
            *ok = true;
        }catch(Exception& e){
            *ok = false;
            *res = e.summary().c_str_dup();
        }catch(std::exception& e){
            *ok = false;
            *res = strdup(e.what());
        }catch(...){
            *ok = false;
            *res = strdup("unknown error");
        }
    }

    void* pkpy_new_repl(void* vm){
        return new REPL((VM*)vm);
    }

    bool pkpy_repl_input(void* r, const char* line){
        return ((REPL*)r)->input(line);
    }

    void pkpy_delete_repl(void* repl){
        delete (REPL*)repl;
    }