#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/common/sstream.h"

typedef c11_vector List;

void py_newlist(py_Ref out) {
    pk_VM* vm = pk_current_vm;
    PyObject* obj = pk_ManagedHeap__gcnew(&vm->heap, tp_list, 0, sizeof(List));
    List* userdata = PyObject__userdata(obj);
    c11_vector__ctor(userdata, sizeof(py_TValue));
    out->type = tp_list;
    out->is_ptr = true;
    out->_obj = obj;
}

void py_newlistn(py_Ref out, int n) {
    py_newlist(out);
    List* userdata = py_touserdata(out);
    c11_vector__reserve(userdata, n);
    userdata->count = n;
}

py_Ref py_list__data(const py_Ref self) {
    List* userdata = py_touserdata(self);
    return userdata->data;
}

py_Ref py_list__getitem(const py_Ref self, int i) {
    List* userdata = py_touserdata(self);
    return c11__at(py_TValue, userdata, i);
}

void py_list__setitem(py_Ref self, int i, const py_Ref val) {
    List* userdata = py_touserdata(self);
    c11__setitem(py_TValue, userdata, i, *val);
}

void py_list__delitem(py_Ref self, int i) {
    List* userdata = py_touserdata(self);
    c11_vector__erase(py_TValue, userdata, i);
}

int py_list__len(const py_Ref self) {
    List* userdata = py_touserdata(self);
    return userdata->count;
}

void py_list__append(py_Ref self, const py_Ref val) {
    List* userdata = py_touserdata(self);
    c11_vector__push(py_TValue, userdata, *val);
}

void py_list__clear(py_Ref self) {
    List* userdata = py_touserdata(self);
    c11_vector__clear(userdata);
}

void py_list__insert(py_Ref self, int i, const py_Ref val) {
    List* userdata = py_touserdata(self);
    c11_vector__insert(py_TValue, userdata, i, *val);
}

////////////////////////////////
static bool _py_list__len__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_i64 res = py_list__len(py_arg(0));
    py_newint(py_retval(), res);
    return true;
}

static bool _py_list__eq__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    if(py_istype(py_arg(1), tp_list)) {
        int length0, length1;
        py_TValue* a0 = pk_arrayview(py_arg(0), &length0);
        py_TValue* a1 = pk_arrayview(py_arg(1), &length1);
        int res = pk_arrayeq(a0, length0, a1, length1);
        if(res == -1) return false;
        py_newbool(py_retval(), res);
    } else {
        py_newnotimplemented(py_retval());
    }
    return true;
}

static bool _py_list__ne__(int argc, py_Ref argv) {
    bool ok = _py_list__eq__(argc, argv);
    if(!ok) return false;
    py_Ref retval = py_retval();
    py_newbool(retval, !py_tobool(retval));
    return true;
}

static bool _py_list__new__(int argc, py_Ref argv) {
    if(argc == 1) {
        py_newlist(py_retval());
        return true;
    }
    if(argc == 2) {
        int length;
        py_TValue* p = pk_arrayview(py_arg(1), &length);
        if(p) {
            py_newlistn(py_retval(), length);
            for(int i = 0; i < length; i++) {
                py_list__setitem(py_retval(), i, p + i);
            }
            return true;
        }
        
        if(!py_iter(py_arg(1))) return false;

        py_Ref iter = py_pushtmp();
        py_Ref list = py_pushtmp();
        *iter = *py_retval();
        py_newlist(list);
        while(true) {
            int res = py_next(iter);
            if(res == -1) return false;
            if(res) {
                py_list__append(list, py_retval());
            } else {
                break;
            }
        }
        *py_retval() = *list;
        return true;
    }
    return TypeError("list() takes at most 1 argument");
}

static bool _py_list__getitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    List* self = py_touserdata(py_arg(0));
    py_Ref _1 = py_arg(1);
    if(_1->type == tp_int) {
        int index = py_toint(py_arg(1));
        if(!pk__normalize_index(&index, self->count)) return false;
        *py_retval() = c11__getitem(py_TValue, self, index);
        return true;
    } else if(_1->type == tp_slice) {
        int start, stop, step;
        bool ok = pk__parse_int_slice(_1, self->count, &start, &stop, &step);
        if(!ok) return false;
        py_newlist(py_retval());
        List* list = py_touserdata(py_retval());
        PK_SLICE_LOOP(i, start, stop, step) {
            c11_vector__push(py_TValue, list, c11__getitem(py_TValue, self, i));
        }
        return true;
    } else {
        return TypeError("list indices must be integers");
    }
}

static bool _py_list__setitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(1, tp_int);
    List* self = py_touserdata(py_arg(0));
    int index = py_toint(py_arg(1));
    if(!pk__normalize_index(&index, self->count)) return false;
    c11__setitem(py_TValue, self, index, *py_arg(2));
    return true;
}

static bool _py_list__delitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_int);
    List* self = py_touserdata(py_arg(0));
    int index = py_toint(py_arg(1));
    if(!pk__normalize_index(&index, self->count)) return false;
    c11_vector__erase(py_TValue, self, index);
    return true;
}

static bool _py_list__add__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_Ref _0 = py_arg(0);
    py_Ref _1 = py_arg(1);
    if(py_istype(_1, tp_list)) {
        List* list_0 = py_touserdata(_0);
        List* list_1 = py_touserdata(_1);
        py_newlist(py_retval());
        List* list = py_touserdata(py_retval());
        c11_vector__extend(py_TValue, list, list_0->data, list_0->count);
        c11_vector__extend(py_TValue, list, list_1->data, list_1->count);
    } else {
        py_newnotimplemented(py_retval());
    }
    return true;
}

static bool _py_list__mul__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_Ref _0 = py_arg(0);
    py_Ref _1 = py_arg(1);
    if(py_istype(_1, tp_int)) {
        int n = py_toint(_1);
        py_newlist(py_retval());
        List* list = py_touserdata(py_retval());
        List* list_0 = py_touserdata(_0);
        for(int i = 0; i < n; i++) {
            c11_vector__extend(py_TValue, list, list_0->data, list_0->count);
        }
    } else {
        py_newnotimplemented(py_retval());
    }
    return true;
}

static bool _py_list__rmul__(int argc, py_Ref argv) { return _py_list__mul__(argc, argv); }

static bool _py_list__append(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_list__append(py_arg(0), py_arg(1));
    py_newnone(py_retval());
    return true;
}

static bool _py_list__repr__(int argc, py_Ref argv) {
    List* self = py_touserdata(py_arg(0));
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    c11_sbuf__write_char(&buf, '[');
    for(int i = 0; i < self->count; i++) {
        py_TValue* val = c11__at(py_TValue, self, i);
        bool ok = py_repr(val);
        if(!ok) {
            c11_sbuf__dtor(&buf);
            return false;
        }
        int size;
        const char* data = py_tostrn(py_retval(), &size);
        c11_sbuf__write_cstrn(&buf, data, size);
        if(i != self->count - 1) c11_sbuf__write_cstr(&buf, ", ");
    }
    c11_sbuf__write_char(&buf, ']');
    c11_string* res = c11_sbuf__submit(&buf);
    py_newstrn(py_retval(), res->data, res->size);
    c11_string__delete(res);
    return true;
}

static bool _py_list__extend(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    List* self = py_touserdata(py_arg(0));
    PY_CHECK_ARG_TYPE(1, tp_list);
    List* other = py_touserdata(py_arg(1));
    c11_vector__extend(py_TValue, self, other->data, other->count);
    py_newnone(py_retval());
    return true;
}

static bool _py_list__count(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    int count = 0;
    for(int i = 0; i < py_list__len(py_arg(0)); i++) {
        int res = py_eq(py_list__getitem(py_arg(0), i), py_arg(1));
        if(res == -1) return false;
        if(res) count++;
    }
    py_newint(py_retval(), count);
    return true;
}

static bool _py_list__clear(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_list__clear(py_arg(0));
    py_newnone(py_retval());
    return true;
}

static bool _py_list__copy(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_newlist(py_retval());
    List* self = py_touserdata(py_arg(0));
    List* list = py_touserdata(py_retval());
    c11_vector__extend(py_TValue, list, self->data, self->count);
    return true;
}

static bool _py_list__index(int argc, py_Ref argv) {
    if(argc > 3) return TypeError("index() takes at most 3 arguments");
    int start = 0;
    if(argc == 3) {
        PY_CHECK_ARG_TYPE(2, tp_int);
        start = py_toint(py_arg(2));
    }
    for(int i = start; i < py_list__len(py_arg(0)); i++) {
        int res = py_eq(py_list__getitem(py_arg(0), i), py_arg(1));
        if(res == -1) return false;
        if(res) {
            py_newint(py_retval(), i);
            return true;
        }
    }
    return ValueError("list.index(x): x not in list");
}

static bool _py_list__reverse(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    List* self = py_touserdata(py_arg(0));
    c11__reverse(py_TValue, self);
    py_newnone(py_retval());
    return true;
}

static bool _py_list__remove(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    for(int i = 0; i < py_list__len(py_arg(0)); i++) {
        int res = py_eq(py_list__getitem(py_arg(0), i), py_arg(1));
        if(res == -1) return false;
        if(res) {
            py_list__delitem(py_arg(0), i);
            py_newnone(py_retval());
            return true;
        }
    }
    return ValueError("list.remove(x): x not in list");
}

static bool _py_list__pop(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    List* self = py_touserdata(py_arg(0));
    if(self->count == 0) return IndexError("pop from empty list");
    *py_retval() = c11_vector__back(py_TValue, self);
    c11_vector__pop(self);
    return true;
}

static bool _py_list__insert(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(1, tp_int);
    List* self = py_touserdata(py_arg(0));
    int index = py_toint(py_arg(1));
    if(index < 0) index += self->count;
    if(index < 0) index = 0;
    if(index > self->count) index = self->count;
    c11_vector__insert(py_TValue, self, index, *py_arg(2));
    py_newnone(py_retval());
    return true;
}

static int _py_lt_with_key(py_TValue* a, py_TValue* b, py_TValue* key) {
    if(!key) return py_lt(a, b);
    pk_VM* vm = pk_current_vm;
    // project a
    py_push(key);
    py_pushnil();
    py_push(a);
    if(!py_vectorcall(1, 0)) return -1;
    py_push(py_retval());
    // project b
    py_push(key);
    py_pushnil();
    py_push(b);
    if(!py_vectorcall(1, 0)) return -1;
    py_push(py_retval());
    // binary op
    bool ok = pk_stack_binaryop(vm, __lt__, __gt__);
    if(!ok) return -1;
    py_shrink(2);
    return py_tobool(py_retval());
}

// sort(self, key=None, reverse=False)
static bool _py_list__sort(int argc, py_Ref argv) {
    List* self = py_touserdata(py_arg(0));

    py_Ref key = py_arg(1);
    if(py_isnone(key)) key = NULL;

    bool ok = c11__stable_sort(self->data,
                               self->count,
                               sizeof(py_TValue),
                               (int (*)(const void*, const void*, void*))_py_lt_with_key,
                               key);
    if(!ok) return false;

    PY_CHECK_ARG_TYPE(2, tp_bool);
    bool reverse = py_tobool(py_arg(2));
    if(reverse) c11__reverse(py_TValue, self);
    py_newnone(py_retval());
    return true;
}

static bool _py_list__iter__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    return py_tpcall(tp_array_iterator, 1, argv);
}

py_Type pk_list__register() {
    pk_VM* vm = pk_current_vm;
    py_Type type = pk_VM__new_type(vm, "list", tp_object, NULL, false);
    pk_TypeInfo* ti = c11__at(pk_TypeInfo, &vm->types, type);
    ti->dtor = (void (*)(void*))c11_vector__dtor;

    py_bindmagic(type, __len__, _py_list__len__);
    py_bindmagic(type, __eq__, _py_list__eq__);
    py_bindmagic(type, __ne__, _py_list__ne__);
    py_bindmagic(type, __new__, _py_list__new__);
    py_bindmagic(type, __getitem__, _py_list__getitem__);
    py_bindmagic(type, __setitem__, _py_list__setitem__);
    py_bindmagic(type, __delitem__, _py_list__delitem__);
    py_bindmagic(type, __add__, _py_list__add__);
    py_bindmagic(type, __mul__, _py_list__mul__);
    py_bindmagic(type, __rmul__, _py_list__rmul__);
    py_bindmagic(type, __repr__, _py_list__repr__);
    py_bindmagic(type, __iter__, _py_list__iter__);

    py_bindmethod(type, "append", _py_list__append);
    py_bindmethod(type, "extend", _py_list__extend);
    py_bindmethod(type, "count", _py_list__count);
    py_bindmethod(type, "clear", _py_list__clear);
    py_bindmethod(type, "copy", _py_list__copy);
    py_bindmethod(type, "index", _py_list__index);
    py_bindmethod(type, "reverse", _py_list__reverse);
    py_bindmethod(type, "remove", _py_list__remove);
    py_bindmethod(type, "pop", _py_list__pop);
    py_bindmethod(type, "insert", _py_list__insert);
    py_bindmethod(type, "sort", _py_list__sort);

    py_bind(py_tpobject(type), "sort(self, key=None, reverse=False)", _py_list__sort);
    return type;
}