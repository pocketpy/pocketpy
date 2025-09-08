#include "pocketpy/common/str.h"
#include "pocketpy/common/vector.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/compiler/compiler.h"

void py_newint(py_OutRef out, py_i64 val) {
    out->type = tp_int;
    out->is_ptr = false;
    out->_i64 = val;
}

void py_newtrivial(py_OutRef out, py_Type type, void* data, int size) {
    out->type = type;
    out->is_ptr = false;
    assert(size <= 16);
    memcpy(&out->_chars, data, size);
}

void py_newfloat(py_OutRef out, py_f64 val) {
    out->type = tp_float;
    out->is_ptr = false;
    out->_f64 = val;
}

void py_newbool(py_OutRef out, bool val) {
    out->type = tp_bool;
    out->is_ptr = false;
    out->_bool = val;
}

void py_newstr(py_OutRef out, const char* data) { py_newstrv(out, (c11_sv){data, strlen(data)}); }

char* py_newstrn(py_OutRef out, int size) {
    if(size < 16) {
        out->type = tp_str;
        out->is_ptr = false;
        c11_string* ud = (c11_string*)(&out->extra);
        c11_string__ctor3(ud, size);
        return ud->data;
    }
    ManagedHeap* heap = &pk_current_vm->heap;
    int total_size = sizeof(c11_string) + size + 1;
    PyObject* obj = ManagedHeap__gcnew(heap, tp_str, 0, total_size);
    c11_string* ud = PyObject__userdata(obj);
    c11_string__ctor3(ud, size);
    out->type = tp_str;
    out->is_ptr = true;
    out->_obj = obj;
    return ud->data;
}

void py_newstrv(py_OutRef out, c11_sv sv) {
    char* data = py_newstrn(out, sv.size);
    memcpy(data, sv.data, sv.size);
}

void py_newfstr(py_OutRef out, const char* fmt, ...) {
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    va_list args;
    va_start(args, fmt);
    pk_vsprintf(&buf, fmt, args);
    va_end(args);
    c11_sbuf__py_submit(&buf, out);
}

unsigned char* py_newbytes(py_OutRef out, int size) {
    ManagedHeap* heap = &pk_current_vm->heap;
    // 4 bytes size + data
    PyObject* obj = ManagedHeap__gcnew(heap, tp_bytes, 0, sizeof(c11_bytes) + size);
    c11_bytes* ud = PyObject__userdata(obj);
    ud->size = size;
    out->type = tp_bytes;
    out->is_ptr = true;
    out->_obj = obj;
    return ud->data;
}

void py_newnone(py_OutRef out) {
    out->type = tp_NoneType;
    out->is_ptr = false;
}

void py_newnotimplemented(py_OutRef out) {
    out->type = tp_NotImplementedType;
    out->is_ptr = false;
}

void py_newellipsis(py_OutRef out) {
    out->type = tp_ellipsis;
    out->is_ptr = false;
}

void py_newnil(py_OutRef out) {
    out->type = tp_nil;
    out->is_ptr = false;
}

void py_newnativefunc(py_OutRef out, py_CFunction f) {
    out->type = tp_nativefunc;
    out->is_ptr = false;
    out->_cfunc = f;
}

py_Name py_newfunction(py_OutRef out,
                       const char* sig,
                       py_CFunction f,
                       const char* docstring,
                       int slots) {
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
    Function__ctor(ud, decl, NULL, NULL);
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

void py_newboundmethod(py_OutRef out, py_Ref self, py_Ref func) {
    py_newobject(out, tp_boundmethod, 2, 0);
    py_setslot(out, 0, self);
    py_setslot(out, 1, func);
}

void* py_newobject(py_OutRef out, py_Type type, int slots, int udsize) {
    ManagedHeap* heap = &pk_current_vm->heap;
    PyObject* obj = ManagedHeap__gcnew(heap, type, slots, udsize);
    out->type = type;
    out->is_ptr = true;
    out->_obj = obj;
    return PyObject__userdata(obj);
}
