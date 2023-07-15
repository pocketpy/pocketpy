#include "pocketpy.h"
#include "pocketpy/pocketpy_c.h"
#include "pocketpy_c.h"

using namespace pkpy;

typedef int (*LuaStyleFuncC)(VM*);

#define PK_ASSERT_N_EXTRA_ELEMENTS(n) \
    int __ex_count = count_extra_elements(vm, n); \
    if(__ex_count < n){ \
        std::string msg = fmt("expected at least ", n, " elements, got ", __ex_count); \
        pkpy_error(vm_handle, "StackError", pkpy_string(msg.c_str())); \
        return false; \
    }

#define PK_ASSERT_NO_ERROR() \
    if(vm->_c.error != nullptr) \
        return false;

static int count_extra_elements(VM* vm, int n){
    if(vm->callstack.empty()){
        return vm->s_data.size();
    }
    PyObject** base = vm->top_frame()->_locals.end();
    return vm->s_data._sp - base;
}

static PyObject* stack_item(VM* vm, int index){
    PyObject** begin;
    PyObject** end;
    if(vm->callstack.empty()){
        begin = vm->s_data.begin();
        end = vm->s_data.end();
    }else{
        Frame* frame = vm->top_frame().get();
        begin = frame->_locals.begin();
        end = frame->_locals.end();
    }
    // may raise
    index = vm->normalized_index(index, end-begin);
    return begin[index];
}

#define PK_PROTECTED(__B) \
    try{ __B }  \
    catch(Exception& e ) { \
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

bool pkpy_exec(pkpy_vm* vm_handle, const char* source) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PyObject* res;
    PK_PROTECTED(
        CodeObject_ code = vm->compile(source, "main.py", EXEC_MODE);
        res = vm->_exec(code, vm->_main);
    )
    return res != nullptr;
}

bool pkpy_exec_2(pkpy_vm* vm_handle, const char* source, const char* filename, int mode, const char* module){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PyObject* res;
    PyObject* mod;
    PK_PROTECTED(
        if(module == nullptr){
            mod = vm->_main;
        }else{
            mod = vm->_modules[module];     // may raise
        }
        CodeObject_ code = vm->compile(source, filename, (CompileMode)mode);
        res = vm->_exec(code, mod);
    )
    return res != nullptr;
}

bool pkpy_pop(pkpy_vm* vm_handle, int n){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_ASSERT_N_EXTRA_ELEMENTS(n)
    vm->s_data.shrink(n);
    return true;
}

bool pkpy_pop_top(pkpy_vm* vm_handle){
    VM* vm = (VM*)vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_ASSERT_N_EXTRA_ELEMENTS(1)
    vm->s_data.pop();
    return true;
}

bool pkpy_dup_top(pkpy_vm* vm_handle){
    VM* vm = (VM*)vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_ASSERT_N_EXTRA_ELEMENTS(1)
    vm->s_data.push(vm->s_data.top());
    return true;
}

bool pkpy_rot_two(pkpy_vm* vm_handle){
    VM* vm = (VM*)vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_ASSERT_N_EXTRA_ELEMENTS(2)
    std::swap(vm->s_data.top(), vm->s_data.second());
    return true;
}

int pkpy_stack_size(pkpy_vm* vm_handle){
    VM* vm = (VM*)vm_handle;
    PK_ASSERT_NO_ERROR()
    if(vm->callstack.empty()){
        return vm->s_data.size();
    }
    return vm->top_frame()->stack_size();
}

// int
bool pkpy_push_int(pkpy_vm* vm_handle, int value) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PyObject* res;
    PK_PROTECTED(
        // int may overflow so we should protect it
        res = py_var(vm, value);
    )
    vm->s_data.push(res);
    return true;
}

bool pkpy_is_int(pkpy_vm* vm_handle, int i){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        return is_int(stack_item(vm, i));
    )
}

bool pkpy_to_int(pkpy_vm* vm_handle, int i, int* out){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        PyObject* item = stack_item(vm, i);
        *out = py_cast<int>(vm, item);
    )
    return true;
}

// float
bool pkpy_push_float(pkpy_vm* vm_handle, float value) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PyObject* res = py_var(vm, value);
    vm->s_data.push(res);
    return true;
}

bool pkpy_is_float(pkpy_vm* vm_handle, int i){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        PyObject* item = stack_item(vm, i);
        return is_float(item);
    )
}

bool pkpy_to_float(pkpy_vm* vm_handle, int i, float* out){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        PyObject* item = stack_item(vm, i);
        *out = py_cast<float>(vm, item);
    )
    return true;
}

// bool
bool pkpy_push_bool(pkpy_vm* vm_handle, bool value) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    vm->s_data.push(value ? vm->True : vm->False);
    return true;
}

bool pkpy_is_bool(pkpy_vm* vm_handle, int i){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        PyObject* item = stack_item(vm, i);
        return is_non_tagged_type(item, vm->tp_bool);
    )
}

bool pkpy_to_bool(pkpy_vm* vm_handle, int i, bool* out){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        PyObject* item = stack_item(vm, i);
        *out = py_cast<bool>(vm, item);
    )
    return true;
}

// string
bool pkpy_push_string(pkpy_vm* vm_handle, pkpy_CString value) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PyObject* res = py_var(vm, std::string_view(value.data, value.size));
    vm->s_data.push(res);
    return true;
}

bool pkpy_is_string(pkpy_vm* vm_handle, int i){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        PyObject* item = stack_item(vm, i);
        return is_non_tagged_type(item, vm->tp_str);
    )
}

bool pkpy_to_string(pkpy_vm* vm_handle, int i, pkpy_CString* out){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        PyObject* item = stack_item(vm, i);
        const Str& s = py_cast<Str&>(vm, item);
        out->data = s.data;
        out->size = s.size;
    )
    return true;
}

// void_p
bool pkpy_push_voidp(pkpy_vm* vm_handle, void* value) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PyObject* res = py_var(vm, value);
    vm->s_data.push(res);
    return true;
}

bool pkpy_is_voidp(pkpy_vm* vm_handle, int i){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        PyObject* item = stack_item(vm, i);
        return is_non_tagged_type(item, VoidP::_type(vm));
    )
}

bool pkpy_to_voidp(pkpy_vm* vm_handle, int i, void** out){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        PyObject* item = stack_item(vm, i);
        VoidP& vp = py_cast<VoidP&>(vm, item);
        *out = vp.ptr;
    )
    return true;
}

// none
bool pkpy_push_none(pkpy_vm* vm_handle) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    vm->s_data.push(vm->None);
    return true;
}

bool pkpy_is_none(pkpy_vm* vm_handle, int i){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        PyObject* item = stack_item(vm, i);
        return item == vm->None;
    )
}

// null
bool pkpy_push_null(pkpy_vm* vm_handle) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    vm->s_data.push(PY_NULL);
    return true;
}

// function
static PyObject* c_function_wrapper(VM* vm, ArgsView args) {
    LuaStyleFuncC f = lambda_get_userdata<LuaStyleFuncC>(args.begin());
    PyObject** curr_sp = vm->s_data._sp;
    int retc = f(vm);
    // propagate_if_errored
    if (vm->_c.error != nullptr){
        Exception e = _py_cast<Exception&>(vm, vm->_c.error);
        vm->_c.error = nullptr;
        vm->_error(e);
        return nullptr;
    }
    PK_ASSERT(retc == vm->s_data._sp-curr_sp);
    if(retc == 0) return vm->None;
    if (retc == 1) return vm->s_data.popx();
    ArgsView ret_view(curr_sp, vm->s_data._sp);
    return py_var(vm, ret_view.to_tuple());
}

bool pkpy_push_function(pkpy_vm* vm_handle, const char* sig, pkpy_CFunction f) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PyObject* f_obj;
    PK_PROTECTED(
        f_obj = vm->bind(
            nullptr,
            sig,
            nullptr,
            c_function_wrapper,
            f
        );
    )
    vm->s_data.push(f_obj);
    return true;
}

// special push
bool pkpy_push_module(pkpy_vm* vm_handle, const char* name) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        PyObject* module = vm->new_module(name);
        vm->s_data.push(module);
    )
    return true;
}

// some opt
bool pkpy_getattr(pkpy_vm* vm_handle, pkpy_CName name) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_ASSERT_N_EXTRA_ELEMENTS(1)
    PyObject* o = vm->s_data.top();
    PK_PROTECTED(
        o = vm->getattr(o, StrName(name));
    )
    vm->s_data.top() = o;
    return true;
}

bool pkpy_setattr(pkpy_vm* vm_handle, pkpy_CName name) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_ASSERT_N_EXTRA_ELEMENTS(2)
    PyObject* a = vm->s_data.top();
    PyObject* val = vm->s_data.second();
    PK_PROTECTED(
        vm->setattr(a, StrName(name), val);
    )
    vm->s_data.shrink(2);
    return true;
}

//get global will also get bulitins
bool pkpy_getglobal(pkpy_vm* vm_handle, pkpy_CName name) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PyObject* o = vm->_main->attr().try_get(StrName(name));
    if (o == nullptr) {
        o = vm->builtins->attr().try_get(StrName(name));
        if (o == nullptr){
            pkpy_error(vm_handle, "NameError", pkpy_name_to_string(name));
            return false;
        }
    }
    vm->s_data.push(o);
    return true;
}

bool pkpy_setglobal(pkpy_vm* vm_handle, pkpy_CName name) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_ASSERT_N_EXTRA_ELEMENTS(1)
    vm->_main->attr().set(StrName(name), vm->s_data.popx());
    return true;
}

bool pkpy_eval(pkpy_vm* vm_handle, const char* source) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_PROTECTED(
        CodeObject_ co = vm->compile(source, "<eval>", EVAL_MODE);
        PyObject* ret = vm->_exec(co, vm->_main);
        vm->s_data.push(ret);
    )
    return true;
}

bool pkpy_unpack_sequence(pkpy_vm* vm_handle, int n) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_ASSERT_N_EXTRA_ELEMENTS(1)
    auto _lock = vm->heap.gc_scope_lock();
    PK_PROTECTED(
        PyObject* _0 = vm->py_iter(vm->s_data.popx());
        for(int i=0; i<n; i++){
            PyObject* _1 = vm->py_next(_0);
            if(_1 == vm->StopIteration) vm->ValueError("not enough values to unpack");
            vm->s_data.push(_1);
        }
        if(vm->py_next(_0) != vm->StopIteration) vm->ValueError("too many values to unpack");
    )
    return true;
}

bool pkpy_get_unbound_method(pkpy_vm* vm_handle, pkpy_CName name){
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    PK_ASSERT_N_EXTRA_ELEMENTS(1)
    PyObject* o = vm->s_data.top();
    PyObject* self;
    PK_PROTECTED(
        o = vm->get_unbound_method(o, StrName(name), &self);
    )
    vm->s_data.pop();
    vm->s_data.push(o);
    vm->s_data.push(self);
    return true;
}

/* Error Handling */
bool pkpy_error(pkpy_vm* vm_handle, const char* name, pkpy_CString message) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_NO_ERROR()
    vm->_c.error = py_var(vm, Exception(name, Str(message.data, message.size)));
    return false;
}

bool pkpy_check_error(pkpy_vm* vm_handle) {
    VM* vm = (VM*) vm_handle;
    return vm->_c.error != nullptr;
}

bool pkpy_clear_error(pkpy_vm* vm_handle, char** message) {
    VM* vm = (VM*) vm_handle;
    // no error
    if (vm->_c.error == nullptr) return false;
    Exception& e = _py_cast<Exception&>(vm, vm->_c.error);
    if (message != nullptr) *message = e.summary().c_str_dup();
    vm->_c.error = nullptr;
    // clear the whole stack??
    vm->callstack.clear();
    vm->s_data.clear(); 
    return true;
}

bool pkpy_vectorcall(pkpy_vm* vm_handle, int argc) {
    VM* vm = (VM*) vm_handle;
    PK_ASSERT_N_EXTRA_ELEMENTS(argc + 2)
    PyObject* res;
    PK_PROTECTED(
        res = vm->vectorcall(argc);
    )
    vm->s_data.push(res);
    return true;
}
/*****************************************************************/
void pkpy_free(void* p){
    free(p);
}

pkpy_CString pkpy_string(const char* value){
    pkpy_CString s;
    s.data = value;
    s.size = strlen(value);
    return s;
}

pkpy_CName pkpy_name(const char* name){
    return StrName(name).index;
}

pkpy_CString pkpy_name_to_string(pkpy_CName name){
    std::string_view sv = StrName(name).sv();
    pkpy_CString s;
    s.data = sv.data();
    s.size = sv.size();
    return s;
}

void pkpy_compile_to_string(pkpy_vm* vm_handle, const char* source, const char* filename, int mode, bool* ok, char** out){
    VM* vm = (VM*) vm_handle;
    try{
        CodeObject_ code = vm->compile(source, filename, (CompileMode)mode);
        *out = code->serialize(vm).c_str_dup();
        *ok = true;
    }catch(Exception& e){
        *ok = false;
        *out = e.summary().c_str_dup();
    }catch(std::exception& e){
        *ok = false;
        *out = strdup(e.what());
    }catch(...){
        *ok = false;
        *out = strdup("unknown error");
    }
}

void* pkpy_new_repl(pkpy_vm* vm_handle){
    return new REPL((VM*)vm_handle);
}

bool pkpy_repl_input(void* r, const char* line){
    return ((REPL*)r)->input(line);
}

void pkpy_delete_repl(void* repl){
    delete (REPL*)repl;
}