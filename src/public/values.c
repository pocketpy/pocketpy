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
    out->type = tp_none_type;
    out->is_ptr = false;
}

void py_newnotimplemented(py_Ref out) {
    out->type = tp_not_implemented_type;
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
    py_bindmethod2(type, name, f, BindType_FUNCTION);
}

void py_bindmethod2(py_Type type, const char* name, py_CFunction f, enum BindType bt) {
    py_TValue tmp;
    py_newnativefunc(&tmp, f);
    py_setdict(py_tpobject(type), py_name(name), &tmp);
}

void py_bindnativefunc(py_Ref obj, const char* name, py_CFunction f) {
    py_TValue tmp;
    py_newnativefunc(&tmp, f);
    py_setdict(obj, py_name(name), &tmp);
}

void py_bind(py_Ref obj, const char* sig, py_CFunction f) {
    py_TValue tmp;
    do{
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "def %s: pass", sig);
        // fn(a, b, *c, d=1) -> None
        CodeObject code;
        pk_SourceData_ source = pk_SourceData__rcnew(buffer, "<bind>", EXEC_MODE, false);
        Error* err = pk_compile(source, &code);
        if(err) abort();
        if(code.func_decls.count != 1) abort();
        FuncDecl_ decl = c11__getitem(FuncDecl_, &code.func_decls, 0);
        // construct the function
        Function* ud = py_newobject(&tmp, tp_function, 0, sizeof(Function));
        Function__ctor(ud, decl, NULL);
        ud->cfunc = f;
        CodeObject__dtor(&code);
        PK_DECREF(source);
    }while(0);
    Function* ud = py_touserdata(&tmp);
    py_Name name = py_name(ud->decl->code.name->data);
    py_setdict(obj, name, &tmp);
}

void* py_newobject(py_Ref out, py_Type type, int slots, int udsize) {
    pk_ManagedHeap* heap = &pk_current_vm->heap;
    PyObject* obj = pk_ManagedHeap__gcnew(heap, type, slots, udsize);
    out->type = type;
    out->is_ptr = true;
    out->_obj = obj;
    return PyObject__userdata(obj);
}
