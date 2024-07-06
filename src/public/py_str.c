#include "pocketpy/common/str.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/common/sstream.h"

void py_newstr(py_Ref out, const char* data) { return py_newstrn(out, data, strlen(data)); }

void py_newstrn(py_Ref out, const char* data, int size) {
    pk_ManagedHeap* heap = &pk_current_vm->heap;
    int total_size = sizeof(c11_string) + size + 1;
    PyObject* obj = pk_ManagedHeap__gcnew(heap, tp_str, 0, total_size);
    c11_string* ud = PyObject__userdata(obj);
    c11_string__ctor2(ud, data, size);
    out->type = tp_str;
    out->is_ptr = true;
    out->_obj = obj;
}

unsigned char* py_newbytes(py_Ref out, int size) {
    pk_ManagedHeap* heap = &pk_current_vm->heap;
    // 4 bytes size + data
    PyObject* obj = pk_ManagedHeap__gcnew(heap, tp_bytes, 0, sizeof(c11_bytes) + size);
    c11_bytes* ud = PyObject__userdata(obj);
    ud->size = size;
    out->type = tp_bytes;
    out->is_ptr = true;
    out->_obj = obj;
    return ud->data;
}

const char* py_tostr(const py_Ref self) {
    assert(self->type == tp_str);
    c11_string* ud = PyObject__userdata(self->_obj);
    return ud->data;
}

const char* py_tostrn(const py_Ref self, int* size) {
    assert(self->type == tp_str);
    c11_string* ud = PyObject__userdata(self->_obj);
    *size = ud->size;
    return ud->data;
}

unsigned char* py_tobytes(const py_Ref self, int* size) {
    assert(self->type == tp_bytes);
    c11_bytes* ud = PyObject__userdata(self->_obj);
    *size = ud->size;
    return ud->data;
}

////////////////////////////////
static bool _py_str__new__(int argc, py_Ref argv) {
    assert(argc >= 1);
    if(argc == 1) {
        py_newstr(py_retval(), "");
        return true;
    }
    if(argc > 2) return TypeError("str() takes at most 1 argument");
    return py_str(py_arg(1));
}

static bool _py_str__hash__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    int size;
    const char* data = py_tostrn(&argv[0], &size);
    py_i64 res = 0;
    for(int i = 0; i < size; i++) {
        res = res * 31 + data[i];
    }
    py_newint(py_retval(), res);
    return true;
}

static bool _py_str__len__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_string* self = py_touserdata(&argv[0]);
    py_newint(py_retval(), self->size);
    return true;
}

static bool _py_str__add__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_string* self = py_touserdata(&argv[0]);
    if(py_arg(1)->type != tp_str) {
        py_newnotimplemented(py_retval());
    } else {
        c11_string* other = py_touserdata(&argv[1]);
        int total_size = sizeof(c11_string) + self->size + other->size + 1;
        c11_string* res = py_newobject(py_retval(), tp_str, 0, total_size);
        res->size = self->size + other->size;
        char* p = (char*)res->data;
        memcpy(p, self->data, self->size);
        memcpy(p + self->size, other->data, other->size);
        p[res->size] = '\0';
    }
    return true;
}

static bool _py_str__mul__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_string* self = py_touserdata(&argv[0]);
    if(py_arg(1)->type != tp_int) {
        py_newnotimplemented(py_retval());
    } else {
        py_i64 n = py_toint(py_arg(1));
        if(n <= 0) {
            py_newstr(py_retval(), "");
        } else {
            int total_size = sizeof(c11_string) + self->size * n + 1;
            c11_string* res = py_newobject(py_retval(), tp_str, 0, total_size);
            res->size = self->size * n;
            char* p = (char*)res->data;
            for(int i = 0; i < n; i++) {
                memcpy(p + i * self->size, self->data, self->size);
            }
            p[res->size] = '\0';
        }
    }
    return true;
}

static bool _py_str__rmul__(int argc, py_Ref argv) { return _py_str__mul__(argc, argv); }

static bool _py_str__contains__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_string* self = py_touserdata(&argv[0]);
    if(py_arg(1)->type != tp_str) {
        py_newnotimplemented(py_retval());
    } else {
        c11_string* other = py_touserdata(&argv[1]);
        const char* p = strstr(self->data, other->data);
        py_newbool(py_retval(), p != NULL);
    }
    return true;
}

static bool _py_str__str__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    *py_retval() = argv[0];
    return true;
}

static bool _py_str__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    int size;
    const char* data = py_tostrn(&argv[0], &size);
    c11_sbuf__write_quoted(&buf, (c11_sv){data, size}, '\'');
    c11_string* res = c11_sbuf__submit(&buf);
    py_newstrn(py_retval(), res->data, res->size);
    c11_string__delete(res);
    return true;
}

static bool _py_str__iter__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    assert(false);
    return false;
}

static bool _py_str__getitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_string* self = py_touserdata(&argv[0]);
    PY_CHECK_ARG_TYPE(1, tp_int);
    c11_sv res = c11_string__u8_getitem(self, py_toint(py_arg(1)));
    py_newstrn(py_retval(), res.data, res.size);
    return true;
}

#define DEF_STR_CMP_OP(op, __f, __cond)                                                            \
    static bool _py_str##op(int argc, py_Ref argv) {                                               \
        PY_CHECK_ARGC(2);                                                                          \
        c11_string* self = py_touserdata(&argv[0]);                                                \
        if(py_arg(1)->type != tp_str) {                                                            \
            py_newnotimplemented(py_retval());                                                     \
        } else {                                                                                   \
            c11_string* other = py_touserdata(&argv[1]);                                           \
            int res = __f(c11_string__sv(self), c11_string__sv(other));                            \
            py_newbool(py_retval(), __cond);                                                       \
        }                                                                                          \
        return true;                                                                               \
    }

DEF_STR_CMP_OP(__eq__, c11__sveq, res)
DEF_STR_CMP_OP(__ne__, c11__sveq, !res)
DEF_STR_CMP_OP(__lt__, c11_sv__cmp, res < 0)
DEF_STR_CMP_OP(__le__, c11_sv__cmp, res <= 0)
DEF_STR_CMP_OP(__gt__, c11_sv__cmp, res > 0)
DEF_STR_CMP_OP(__ge__, c11_sv__cmp, res >= 0)

#undef DEF_STR_CMP_OP

static bool _py_str__lower(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_string* self = py_touserdata(&argv[0]);
    int total_size = sizeof(c11_string) + self->size + 1;
    c11_string* res = py_newobject(py_retval(), tp_str, 0, total_size);
    res->size = self->size;
    char* p = (char*)res->data;
    for(int i = 0; i < self->size; i++) {
        char c = self->data[i];
        p[i] = c >= 'A' && c <= 'Z' ? c + 32 : c;
    }
    p[res->size] = '\0';
    return true;
}

static bool _py_str__upper(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_string* self = py_touserdata(&argv[0]);
    int total_size = sizeof(c11_string) + self->size + 1;
    c11_string* res = py_newobject(py_retval(), tp_str, 0, total_size);
    res->size = self->size;
    char* p = (char*)res->data;
    for(int i = 0; i < self->size; i++) {
        char c = self->data[i];
        p[i] = c >= 'a' && c <= 'z' ? c - 32 : c;
    }
    p[res->size] = '\0';
    return true;
}

static bool _py_str__startswith(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_string* self = py_touserdata(&argv[0]);
    PY_CHECK_ARG_TYPE(1, tp_str);
    c11_string* other = py_touserdata(&argv[1]);
    c11_sv _0 = c11_sv__slice2(c11_string__sv(self), 0, other->size);
    c11_sv _1 = c11_string__sv(other);
    py_newbool(py_retval(), c11__sveq(_0, _1));
    return true;
}

static bool _py_str__endswith(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_string* self = py_touserdata(&argv[0]);
    PY_CHECK_ARG_TYPE(1, tp_str);
    c11_string* other = py_touserdata(&argv[1]);
    c11_sv _0 = c11_sv__slice2(c11_string__sv(self), self->size - other->size, self->size);
    c11_sv _1 = c11_string__sv(other);
    py_newbool(py_retval(), c11__sveq(_0, _1));
    return true;
}

static bool _py_str__join(int argc, py_Ref argv) {
    assert(false);
    // PY_CHECK_ARGC(2);
    // c11_sbuf buf;
    // c11_sbuf__ctor(&buf);
    // c11_string* sep = py_touserdata(&argv[0]);
    // py_Ref iter = py_pushtmp();
    // py_iter(iter, &argv[1]);
    return false;
}

py_Type pk_str__register() {
    pk_VM* vm = pk_current_vm;
    py_Type type = pk_VM__new_type(vm, "str", tp_object, NULL, false);
    // no need to dtor because the memory is controlled by the object

    py_bindmagic(tp_str, __new__, _py_str__new__);
    py_bindmagic(tp_str, __hash__, _py_str__hash__);
    py_bindmagic(tp_str, __len__, _py_str__len__);
    py_bindmagic(tp_str, __add__, _py_str__add__);
    py_bindmagic(tp_str, __mul__, _py_str__mul__);
    py_bindmagic(tp_str, __rmul__, _py_str__rmul__);
    py_bindmagic(tp_str, __contains__, _py_str__contains__);
    py_bindmagic(tp_str, __str__, _py_str__str__);
    py_bindmagic(tp_str, __repr__, _py_str__repr__);
    py_bindmagic(tp_str, __iter__, _py_str__iter__);
    py_bindmagic(tp_str, __getitem__, _py_str__getitem__);

    py_bindmagic(tp_str, __eq__, _py_str__eq__);
    py_bindmagic(tp_str, __ne__, _py_str__ne__);
    py_bindmagic(tp_str, __lt__, _py_str__lt__);
    py_bindmagic(tp_str, __le__, _py_str__le__);
    py_bindmagic(tp_str, __gt__, _py_str__gt__);
    py_bindmagic(tp_str, __ge__, _py_str__ge__);

    py_bindmethod(tp_str, "lower", _py_str__lower);
    py_bindmethod(tp_str, "upper", _py_str__upper);
    py_bindmethod(tp_str, "startswith", _py_str__startswith);
    py_bindmethod(tp_str, "endswith", _py_str__endswith);
    py_bindmethod(tp_str, "join", _py_str__join);
    return type;
}

py_Type pk_bytes__register() {
    pk_VM* vm = pk_current_vm;
    py_Type type = pk_VM__new_type(vm, "bytes", tp_object, NULL, false);
    // no need to dtor because the memory is controlled by the object
    return type;
}

bool py_str(py_Ref val) {
    py_Ref tmp = py_tpfindmagic(val->type, __str__);
    if(!tmp) return py_repr(val);
    return py_call(tmp, 1, val);
}
