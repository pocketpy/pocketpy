#include "pocketpy/common/str.h"
#include "pocketpy/common/vector.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/compiler/compiler.h"

void py_newint(py_Ref out, int64_t val) {
    out->type = tp_int;
    out->is_ptr = false;
    out->_i64 = val;
}

void py_newfloat(py_Ref out, double val) {
    out->type = tp_float;
    out->is_ptr = false;
    out->_f64 = val;
}

void py_newbool(py_Ref out, bool val) {
    out->type = tp_bool;
    out->is_ptr = false;
    out->_bool = val;
}

void py_newnone(py_Ref out) {
    out->type = tp_NoneType;
    out->is_ptr = false;
}

void py_newnotimplemented(py_Ref out) {
    out->type = tp_NotImplementedType;
    out->is_ptr = false;
}

void py_newellipsis(py_Ref out) {
    out->type = tp_ellipsis;
    out->is_ptr = false;
}

void py_newnil(py_Ref out) { out->type = 0; }

void py_newnativefunc(py_Ref out, py_CFunction f) {
    out->type = tp_nativefunc;
    out->is_ptr = false;
    out->_cfunc = f;
}

void py_bindmethod(py_Type type, const char* name, py_CFunction f) {
    py_TValue tmp;
    py_newnativefunc(&tmp, f);
    py_setdict(py_tpobject(type), py_name(name), &tmp);
}

void py_bindfunc(py_Ref obj, const char* name, py_CFunction f) {
    py_TValue tmp;
    py_newnativefunc(&tmp, f);
    py_setdict(obj, py_name(name), &tmp);
}

void py_bindproperty(py_Type type, const char* name, py_CFunction getter, py_CFunction setter) {
    py_TValue tmp;
    py_newobject(&tmp, tp_property, 2, 0);
    py_newnativefunc(py_getslot(&tmp, 0), getter);
    if(setter) {
        py_newnativefunc(py_getslot(&tmp, 1), setter);
    } else {
        py_setslot(&tmp, 1, py_None());
    }
    py_setdict(py_tpobject(type), py_name(name), &tmp);
}

void py_bind(py_Ref obj, const char* sig, py_CFunction f) {
    py_TValue tmp;
    py_Name name = py_newfunction(&tmp, sig, f, NULL, 0);
    py_setdict(obj, name, &tmp);
}

py_Name
    py_newfunction(py_Ref out, const char* sig, py_CFunction f, const char* docstring, int slots) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "def %s: pass", sig);
    // fn(a, b, *c, d=1) -> None
    CodeObject code;
    SourceData_ source = SourceData__rcnew(buffer, "<bind>", EXEC_MODE, false);
    Error* err = pk_compile(source, &code);
    if(err || code.func_decls.length != 1) {
        c11__abort("py_newfunction(): invalid signature '%s'", sig);
    }
    FuncDecl_ decl = c11__getitem(FuncDecl_, &code.func_decls, 0);
    decl->docstring = docstring;
    // construct the function
    Function* ud = py_newobject(out, tp_function, slots, sizeof(Function));
    Function__ctor(ud, decl, NULL);
    ud->cfunc = f;
    CodeObject__dtor(&code);
    PK_DECREF(source);
    assert(decl->rc.count == 1);
    py_Name decl_name = py_name(ud->decl->code.name->data);
    if(decl_name == __new__ || decl_name == __init__) {
        if(ud->decl->args.length == 0) {
            c11__abort("%s() should have at least one positional argument", py_name2str(decl_name));
        }
    }
    return decl_name;
}

void py_newboundmethod(py_Ref out, py_Ref self, py_Ref func) {
    py_newobject(out, tp_boundmethod, 2, 0);
    py_setslot(out, 0, self);
    py_setslot(out, 1, func);
}

void* py_newobject(py_Ref out, py_Type type, int slots, int udsize) {
    ManagedHeap* heap = &pk_current_vm->heap;
    PyObject* obj = ManagedHeap__gcnew(heap, type, slots, udsize);
    out->type = type;
    out->is_ptr = true;
    out->_obj = obj;
    return PyObject__userdata(obj);
}
