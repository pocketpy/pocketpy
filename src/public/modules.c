#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/vm.h"

py_Ref py_getmodule(const char* name) {
    pk_VM* vm = pk_current_vm;
    return pk_NameDict__try_get(&vm->modules, py_name(name));
}

py_Ref py_newmodule(const char* name, const char* package) {
    pk_ManagedHeap* heap = &pk_current_vm->heap;
    PyObject* obj = pk_ManagedHeap__gcnew(heap, tp_module, -1, 0);

    py_Ref r0 = py_pushtmp();
    py_Ref r1 = py_pushtmp();

    *r0 = (py_TValue){
        .type = obj->type,
        .is_ptr = true,
        ._obj = obj,
    };

    py_newstr(r1, name);
    py_setdict(r0, __name__, r1);

    package = package ? package : "";

    py_newstr(r1, package);
    py_setdict(r0, __package__, r1);

    // convert to fullname
    if(package[0] != '\0') {
        // package.name
        char buf[256];
        snprintf(buf, sizeof(buf), "%s.%s", package, name);
        name = buf;
    }

    py_newstr(r1, name);
    py_setdict(r0, __path__, r1);

    // we do not allow override in order to avoid memory leak
    // it is because Module objects are not garbage collected
    bool exists = pk_NameDict__contains(&pk_current_vm->modules, py_name(name));
    if(exists) abort();
    pk_NameDict__set(&pk_current_vm->modules, py_name(name), *r0);

    py_poptmp(2);
    return py_getmodule(name);
}

//////////////////////////

static bool _py_builtins__repr(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    return py_repr(argv);
}

static bool _py_builtins__exit(int argc, py_Ref argv) {
    int code = 0;
    if(argc > 1) return TypeError("exit() takes at most 1 argument");
    if(argc == 1) {
        PY_CHECK_ARG_TYPE(0, tp_int);
        code = py_toint(argv);
    }
    // return py_exception("SystemExit", "%d", code);
    exit(code);
    return false;
}

static bool _py_builtins__len(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    return py_len(argv);
}

static bool _py_builtins__hex(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_int);

    py_i64 val = py_toint(argv);

    if(val == 0) {
        py_newstr(py_retval(), "0x0");
        return true;
    }

    c11_sbuf ss;
    c11_sbuf__ctor(&ss);

    if(val < 0) {
        c11_sbuf__write_char(&ss, '-');
        val = -val;
    }
    c11_sbuf__write_cstr(&ss, "0x");
    bool non_zero = true;
    for(int i = 56; i >= 0; i -= 8) {
        unsigned char cpnt = (val >> i) & 0xff;
        c11_sbuf__write_hex(&ss, cpnt, non_zero);
        if(cpnt != 0) non_zero = false;
    }
    // return VAR(ss.str());
    c11_string* res = c11_sbuf__submit(&ss);
    py_newstrn(py_retval(), res->data, res->size);
    c11_string__delete(res);
    return true;
}

static bool _py_builtins__iter(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    return py_iter(argv);
}

static bool _py_builtins__next(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    int res = py_next(argv);
    if(res == -1) return false;
    if(res) {
        return true;
    } else {
        return StopIteration();
    }
}

py_TValue pk_builtins__register() {
    py_Ref builtins = py_newmodule("builtins", NULL);
    py_bindnativefunc(builtins, "repr", _py_builtins__repr);
    py_bindnativefunc(builtins, "exit", _py_builtins__exit);
    py_bindnativefunc(builtins, "len", _py_builtins__len);
    py_bindnativefunc(builtins, "hex", _py_builtins__hex);
    py_bindnativefunc(builtins, "iter", _py_builtins__iter);
    py_bindnativefunc(builtins, "next", _py_builtins__next);
    return *builtins;
}

py_Type pk_function__register() {
    pk_VM* vm = pk_current_vm;
    py_Type type = pk_VM__new_type(vm, "function", tp_object, NULL, false);
    pk_TypeInfo* ti = c11__at(pk_TypeInfo, &vm->types, type);
    ti->dtor = (void (*)(void*))Function__dtor;
    return type;
}

static bool _py_nativefunc__repr(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_newstr(py_retval(), "<nativefunc object>");
    return true;
}

py_Type pk_nativefunc__register() {
    pk_VM* vm = pk_current_vm;
    py_Type type = pk_VM__new_type(vm, "nativefunc", tp_object, NULL, false);
    py_bindmagic(type, __repr__, _py_nativefunc__repr);
    return type;
}