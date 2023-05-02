#include "pocketpy.h"
#include "pocketpy_c.h"

using namespace pkpy;

#define PKPY_STACK_SIZE 32

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
    if (w->c_data->size() > 0 && w->c_data->top() == nullptr) \
        return false; \

#define ERRHANDLER_CLOSE \
    } catch( Exception e ) { \
        w->c_data->push(py_var(w->vm, e)); \
        w->c_data->push(NULL); \
        return false; \
    } \
    SAFEGUARD_CLOSE \

struct pkpy_vm_wrapper {
    VM* vm;
    ValueStackImpl<PKPY_STACK_SIZE>* c_data;
    char* string_ret;
};


//for now I will unpack a tuple automatically, we may not want to handle
//it this way, not sure
//it is more lua like, but maybe not python like
static void unpack_return(struct pkpy_vm_wrapper* w, PyObject* ret) {
    if (is_type(ret, w->vm->tp_tuple)) {
        Tuple& t = py_cast<Tuple&>(w->vm, ret);
        for (int i = 0; i < t.size(); i++) 
            w->c_data->push(t[i]);
    } else if (ret == w->vm->None) {
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
        w->c_data->push(ret);

}

static char* manage_string(struct pkpy_vm_wrapper* w, char* s) {
    if (w->string_ret != NULL)
        free(w->string_ret);
    w->string_ret = s;
    return w->string_ret;
}


bool pkpy_clear_error(struct pkpy_vm_wrapper* w, char** message) {
    SAFEGUARD_OPEN

        if (w->c_data->size() == 0 || w->c_data->top() != nullptr) 
            return false;

        w->c_data->pop();
        Exception& e = py_cast<Exception&>(w->vm, w->c_data->top());
        if (message != nullptr) 
            *message = manage_string(w, e.summary().c_str_dup());
        else
            std::cerr << "ERROR: " << e.summary() << "\n";

        w->c_data->clear();
        w->vm->callstack.clear();
        w->vm->s_data.clear(); 
        return true;

    SAFEGUARD_CLOSE
}

struct pkpy_vm_wrapper* pkpy_vm_create(bool use_stdio, bool enable_os) {
    struct pkpy_vm_wrapper* w = (struct pkpy_vm_wrapper*) malloc(sizeof(*w));
    w->vm = new VM(use_stdio, enable_os);
    w->c_data = new ValueStackImpl<PKPY_STACK_SIZE>();
    w->string_ret = NULL;

    return w;
}

bool pkpy_vm_run(struct pkpy_vm_wrapper* w, const char* source) {
    ERRHANDLER_OPEN

    CodeObject_ code = w->vm->compile(source, "<c-bound>", EXEC_MODE);
    PyObject* result = w->vm->_exec(code, w->vm->_main);

    //unpack_return(w, result);
    //NOTE: it seems like w->vm->_exec should return whatever the last command it
    //ran returned but instead it seems to pretty much always return None
    //so I guess uncomment this line if that every changes

    return true;
    ERRHANDLER_CLOSE
}

void pkpy_vm_destroy(struct pkpy_vm_wrapper* w) {
    delete w->vm;
    delete w->c_data;
    if (w->string_ret != NULL)
        free(w->string_ret);
    free(w);
}

static void propagate_if_errored(struct pkpy_vm_wrapper* w) {
    try {
        if (w->c_data->size() == 0 || w->c_data->top() != nullptr) 
            return;

        w->c_data->pop();
        Exception& e = py_cast<Exception&>(w->vm, w->c_data->top());
        w->c_data->pop();

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
    LuaStyleFuncC f = py_cast<NativeFunc&>(vm, args[-2])._lua_f;

    //setup c stack

    struct pkpy_vm_wrapper w;

    ValueStackImpl c_stack = ValueStackImpl<PKPY_STACK_SIZE>();
    w.vm = vm;
    w.c_data = &c_stack;

    for (int i = 0; i < args.size(); i++)
        w.c_data->push(args[i]);
    
    int retc = f(&w);

    PyObject* ret = w.vm->None;
    propagate_if_errored(&w);

    if (retc == 1) 
        ret = w.c_data->top();
    else if (retc > 1) {
        Tuple t = Tuple(retc);

        for (int i = 0; i < retc; i++)  {
            int stack_index = (w.c_data->size() - retc) + i;
            t[i] = w.c_data->begin()[stack_index];
        }

        ret = py_var(w.vm, t);
    }

    return ret;
}

bool pkpy_push_function(struct pkpy_vm_wrapper* w, pkpy_function f) {
    ERRHANDLER_OPEN

    //TODO right now we just treat all c bound functions a varargs functions
    //do we want to change that?
    NativeFunc nf = NativeFunc(c_function_wrapper, -1, 0);
    nf._lua_f = (LuaStyleFuncC) f;

    w->c_data->push(py_var(w->vm, nf));

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_push_int(struct pkpy_vm_wrapper* w, int value) {
    ERRHANDLER_OPEN

    w->c_data->push(py_var(w->vm, value));

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_push_float(struct pkpy_vm_wrapper* w, double value) {
    ERRHANDLER_OPEN
    w->c_data->push(py_var(w->vm, value));

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_push_bool(struct pkpy_vm_wrapper* w, bool value) {
    ERRHANDLER_OPEN
    w->c_data->push(py_var(w->vm, value));

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_push_string(struct pkpy_vm_wrapper* w, const char* value) {
    ERRHANDLER_OPEN
    w->c_data->push(py_var(w->vm, value));

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_push_stringn(struct pkpy_vm_wrapper* w, const char* value, int length) {
    ERRHANDLER_OPEN

    Str s = Str(value, length);
    w->c_data->push(py_var(w->vm, s));

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_push_none(struct pkpy_vm_wrapper* w) {
    ERRHANDLER_OPEN
    w->c_data->push(w->vm->None);

    return true;
    ERRHANDLER_CLOSE
}



bool pkpy_set_global(struct pkpy_vm_wrapper* w, const char* name) {
    ERRHANDLER_OPEN

    w->vm->_main->attr().set(name, w->c_data->top());

    w->c_data->pop();

    return true;
    ERRHANDLER_CLOSE
}

//get global will also get bulitins
bool pkpy_get_global(struct pkpy_vm_wrapper* w, const char* name) {
    ERRHANDLER_OPEN

    PyObject* o = w->vm->_main->attr().try_get(name);
    if (o == nullptr) {
        o = w->vm->builtins->attr().try_get(name);
        if (o == nullptr)
            throw Exception("AttributeError", "could not find requested global");
    }

    w->c_data->push(o);

    return true;
    ERRHANDLER_CLOSE
}


bool pkpy_call(struct pkpy_vm_wrapper* w, int argc) {
    ERRHANDLER_OPEN

    int callable_index = w->c_data->size() - argc  - 1;

    PyObject* callable = w->c_data->begin()[callable_index];

    w->vm->s_data.push(callable);
    w->vm->s_data.push(PY_NULL);

    for (int i = 0; i < argc; i++) 
        w->vm->s_data.push(w->c_data->begin()[callable_index + i + 1]);

    PyObject* o = w->vm->vectorcall(argc);

    w->c_data->shrink(argc + 1);

    unpack_return(w, o);

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_call_method(struct pkpy_vm_wrapper* w, const char* name, int argc) {
    ERRHANDLER_OPEN

    int self_index = w->c_data->size() - argc  - 1;
    PyObject* self = w->c_data->begin()[self_index];

    PyObject* callable = w->vm->get_unbound_method(self, name, &self);

    w->vm->s_data.push(callable);
    w->vm->s_data.push(self);

    for (int i = 0; i < argc; i++) 
        w->vm->s_data.push(w->c_data->begin()[self_index + i + 1]);

    PyObject* o = w->vm->vectorcall(argc);

    w->c_data->shrink(argc + 1);

    unpack_return(w, o);

    return true;
    ERRHANDLER_CLOSE
}



static int lua_to_cstack_index(int index, int size) {
    if (index < 0)
        index = size + index;
    return index;
}

bool pkpy_to_int(struct pkpy_vm_wrapper* w, int index, int* ret) {
    ERRHANDLER_OPEN

    index = lua_to_cstack_index(index, w->c_data->size());

    PyObject* o = w->c_data->begin()[index];
    if (ret != nullptr)
        *ret = py_cast<int>(w->vm, o);

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_to_float(struct pkpy_vm_wrapper* w, int index, double* ret) {
    ERRHANDLER_OPEN

    index = lua_to_cstack_index(index, w->c_data->size());

    PyObject* o = w->c_data->begin()[index];
    if (ret != nullptr)
        *ret = py_cast<double>(w->vm, o);

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_to_bool(struct pkpy_vm_wrapper* w, int index, bool* ret) {
    ERRHANDLER_OPEN

    index = lua_to_cstack_index(index, w->c_data->size());

    PyObject* o = w->c_data->begin()[index];
    if (ret != nullptr)
        *ret = py_cast<bool>(w->vm, o);

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_to_string(struct pkpy_vm_wrapper* w, int index, char** ret) {
    ERRHANDLER_OPEN

    index = lua_to_cstack_index(index, w->c_data->size());

    PyObject* o = w->c_data->begin()[index];
    if (ret != nullptr) {
        Str& s = py_cast<Str&>(w->vm, o);
        *ret = manage_string(w, s.c_str_dup());
    }

    return true;
    ERRHANDLER_CLOSE
}

bool pkpy_is_int(struct pkpy_vm_wrapper* w, int index) {
    index = lua_to_cstack_index(index, w->c_data->size());
    PyObject* o = w->c_data->begin()[index];

    return is_type(o, w->vm->tp_int);
}
bool pkpy_is_float(struct pkpy_vm_wrapper* w, int index) {
    index = lua_to_cstack_index(index, w->c_data->size());
    PyObject* o = w->c_data->begin()[index];

    return is_type(o, w->vm->tp_float);
}
bool pkpy_is_bool(struct pkpy_vm_wrapper* w, int index) {
    index = lua_to_cstack_index(index, w->c_data->size());
    PyObject* o = w->c_data->begin()[index];

    return is_type(o, w->vm->tp_bool);
}
bool pkpy_is_string(struct pkpy_vm_wrapper* w, int index) {
    index = lua_to_cstack_index(index, w->c_data->size());
    PyObject* o = w->c_data->begin()[index];

    return is_type(o, w->vm->tp_str);
}
bool pkpy_is_none(struct pkpy_vm_wrapper* w, int index) {
    index = lua_to_cstack_index(index, w->c_data->size());
    PyObject* o = w->c_data->begin()[index];

    return o == w->vm->None;
}

bool pkpy_check_stack(struct pkpy_vm_wrapper* w, int free) {
    return free + w->c_data->size() <= PKPY_STACK_SIZE;
}

int pkpy_stack_size(struct pkpy_vm_wrapper* w) {
    return w->c_data->size();
}

bool pkpy_pop(struct pkpy_vm_wrapper* w, int n) {
    w->c_data->shrink(n);
    return true;
}
