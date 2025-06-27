#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/interpreter/types.h"
#include "pocketpy/objects/iterator.h"
#include "pocketpy/common/sstream.h"

void py_newlist(py_OutRef out) {
    List* ud = py_newobject(out, tp_list, 0, sizeof(List));
    c11_vector__ctor(ud, sizeof(py_TValue));
}

void py_newlistn(py_OutRef out, int n) {
    py_newlist(out);
    List* ud = py_touserdata(out);
    c11_vector__reserve(ud, n);
    ud->length = n;
}

py_Ref py_list_data(py_Ref self) {
    List* ud = py_touserdata(self);
    return ud->data;
}

py_Ref py_list_getitem(py_Ref self, int i) {
    List* ud = py_touserdata(self);
    return c11__at(py_TValue, ud, i);
}

void py_list_setitem(py_Ref self, int i, py_Ref val) {
    List* ud = py_touserdata(self);
    c11__setitem(py_TValue, ud, i, *val);
}

void py_list_delitem(py_Ref self, int i) {
    List* ud = py_touserdata(self);
    c11_vector__erase(py_TValue, ud, i);
}

int py_list_len(py_Ref self) {
    List* ud = py_touserdata(self);
    return ud->length;
}

void py_list_swap(py_Ref self, int i, int j) {
    py_TValue* data = py_list_data(self);
    py_TValue tmp = data[i];
    data[i] = data[j];
    data[j] = tmp;
}

void py_list_append(py_Ref self, py_Ref val) {
    List* ud = py_touserdata(self);
    c11_vector__push(py_TValue, ud, *val);
}

py_ItemRef py_list_emplace(py_Ref self) {
    List* ud = py_touserdata(self);
    c11_vector__emplace(ud);
    return &c11_vector__back(py_TValue, ud);
}

void py_list_clear(py_Ref self) {
    List* ud = py_touserdata(self);
    c11_vector__clear(ud);
}

void py_list_insert(py_Ref self, int i, py_Ref val) {
    List* ud = py_touserdata(self);
    c11_vector__insert(py_TValue, ud, i, *val);
}

////////////////////////////////
static bool list__len__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_i64 res = py_list_len(py_arg(0));
    py_newint(py_retval(), res);
    return true;
}

static bool list__eq__(int argc, py_Ref argv) {
    return pk_wrapper__arrayequal(tp_list, argc, argv);
}

static bool list__ne__(int argc, py_Ref argv) {
    if(!list__eq__(argc, argv)) return false;
    if(py_isbool(py_retval())) {
        bool res = py_tobool(py_retval());
        py_newbool(py_retval(), !res);
    }
    return true;
}

static bool list__new__(int argc, py_Ref argv) {
    if(argc == 1) {
        py_newlist(py_retval());
        return true;
    }
    if(argc == 2) {
        py_TValue* p;
        int length = pk_arrayview(py_arg(1), &p);
        if(length != -1) {
            py_newlistn(py_retval(), length);
            for(int i = 0; i < length; i++) {
                py_list_setitem(py_retval(), i, p + i);
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
            if(res == -1) {
                py_shrink(2);
                return false;
            }
            if(!res) break;
            py_list_append(list, py_retval());
        }
        *py_retval() = *list;
        py_shrink(2);
        return true;
    }
    return TypeError("list() takes at most 1 argument");
}

static bool list__getitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    List* self = py_touserdata(py_arg(0));
    py_Ref _1 = py_arg(1);
    if(_1->type == tp_int) {
        int index = py_toint(py_arg(1));
        if(!pk__normalize_index(&index, self->length)) return false;
        *py_retval() = c11__getitem(py_TValue, self, index);
        return true;
    } else if(_1->type == tp_slice) {
        int start, stop, step;
        bool ok = pk__parse_int_slice(_1, self->length, &start, &stop, &step);
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

static bool list__setitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(1, tp_int);
    List* self = py_touserdata(py_arg(0));
    int index = py_toint(py_arg(1));
    if(!pk__normalize_index(&index, self->length)) return false;
    c11__setitem(py_TValue, self, index, *py_arg(2));
    py_newnone(py_retval());
    return true;
}

static bool list__delitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_int);
    List* self = py_touserdata(py_arg(0));
    int index = py_toint(py_arg(1));
    if(!pk__normalize_index(&index, self->length)) return false;
    c11_vector__erase(py_TValue, self, index);
    py_newnone(py_retval());
    return true;
}

static bool list__add__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_Ref _0 = py_arg(0);
    py_Ref _1 = py_arg(1);
    if(py_istype(_1, tp_list)) {
        List* list_0 = py_touserdata(_0);
        List* list_1 = py_touserdata(_1);
        py_newlist(py_retval());
        List* list = py_touserdata(py_retval());
        c11_vector__extend(py_TValue, list, list_0->data, list_0->length);
        c11_vector__extend(py_TValue, list, list_1->data, list_1->length);
    } else {
        py_newnotimplemented(py_retval());
    }
    return true;
}

static bool list__mul__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_Ref _0 = py_arg(0);
    py_Ref _1 = py_arg(1);
    if(py_istype(_1, tp_int)) {
        int n = py_toint(_1);
        py_newlist(py_retval());
        List* list = py_touserdata(py_retval());
        List* list_0 = py_touserdata(_0);
        for(int i = 0; i < n; i++) {
            c11_vector__extend(py_TValue, list, list_0->data, list_0->length);
        }
    } else {
        py_newnotimplemented(py_retval());
    }
    return true;
}

static bool list__rmul__(int argc, py_Ref argv) { return list__mul__(argc, argv); }

static bool list_append(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_list_append(py_arg(0), py_arg(1));
    py_newnone(py_retval());
    return true;
}

static bool list__repr__(int argc, py_Ref argv) {
    List* self = py_touserdata(py_arg(0));
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    c11_sbuf__write_char(&buf, '[');
    for(int i = 0; i < self->length; i++) {
        py_TValue* val = c11__at(py_TValue, self, i);
        bool ok = py_repr(val);
        if(!ok) {
            c11_sbuf__dtor(&buf);
            return false;
        }
        c11_sbuf__write_sv(&buf, py_tosv(py_retval()));
        if(i != self->length - 1) c11_sbuf__write_cstr(&buf, ", ");
    }
    c11_sbuf__write_char(&buf, ']');
    c11_sbuf__py_submit(&buf, py_retval());
    return true;
}

static bool list_extend(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    List* self = py_touserdata(py_arg(0));
    py_TValue* p;
    int length = pk_arrayview(py_arg(1), &p);
    if(length == -1) return TypeError("extend() argument must be a list or tuple");
    c11_vector__extend(py_TValue, self, p, length);
    py_newnone(py_retval());
    return true;
}

static bool list_count(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    int count = 0;
    for(int i = 0; i < py_list_len(py_arg(0)); i++) {
        int res = py_equal(py_list_getitem(py_arg(0), i), py_arg(1));
        if(res == -1) return false;
        if(res) count++;
    }
    py_newint(py_retval(), count);
    return true;
}

static bool list_clear(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_list_clear(py_arg(0));
    py_newnone(py_retval());
    return true;
}

static bool list_copy(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_newlist(py_retval());
    List* self = py_touserdata(py_arg(0));
    List* list = py_touserdata(py_retval());
    c11_vector__extend(py_TValue, list, self->data, self->length);
    return true;
}

static bool list_index(int argc, py_Ref argv) {
    if(argc > 3) return TypeError("index() takes at most 3 arguments");
    int start = 0;
    if(argc == 3) {
        PY_CHECK_ARG_TYPE(2, tp_int);
        start = py_toint(py_arg(2));
    }
    for(int i = start; i < py_list_len(py_arg(0)); i++) {
        int res = py_equal(py_list_getitem(py_arg(0), i), py_arg(1));
        if(res == -1) return false;
        if(res) {
            py_newint(py_retval(), i);
            return true;
        }
    }
    return ValueError("list.index(x): x not in list");
}

static bool list_reverse(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    List* self = py_touserdata(py_arg(0));
    c11__reverse(py_TValue, self);
    py_newnone(py_retval());
    return true;
}

static bool list_remove(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    for(int i = 0; i < py_list_len(py_arg(0)); i++) {
        int res = py_equal(py_list_getitem(py_arg(0), i), py_arg(1));
        if(res == -1) return false;
        if(res) {
            py_list_delitem(py_arg(0), i);
            py_newnone(py_retval());
            return true;
        }
    }
    return ValueError("list.remove(x): x not in list");
}

static bool list_pop(int argc, py_Ref argv) {
    int index;
    if(argc == 1) {
        index = -1;
    } else if(argc == 2) {
        PY_CHECK_ARG_TYPE(1, tp_int);
        index = py_toint(py_arg(1));
    } else {
        return TypeError("pop() takes at most 2 arguments");
    }
    List* self = py_touserdata(py_arg(0));
    if(self->length == 0) return IndexError("pop from empty list");
    if(!pk__normalize_index(&index, self->length)) return false;
    *py_retval() = c11__getitem(py_TValue, self, index);
    c11_vector__erase(py_TValue, self, index);
    return true;
}

static bool list_insert(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(1, tp_int);
    List* self = py_touserdata(py_arg(0));
    int index = py_toint(py_arg(1));
    if(index < 0) index += self->length;
    if(index < 0) index = 0;
    if(index > self->length) index = self->length;
    c11_vector__insert(py_TValue, self, index, *py_arg(2));
    py_newnone(py_retval());
    return true;
}

static int lt_with_key(py_TValue* a, py_TValue* b, py_TValue* key) {
    if(!key) return py_less(a, b);
    VM* vm = pk_current_vm;
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
    return py_bool(py_retval());
}

// sort(self, key=None, reverse=False)
static bool list_sort(int argc, py_Ref argv) {
    List* self = py_touserdata(py_arg(0));

    py_Ref key = py_arg(1);
    if(py_isnone(key)) key = NULL;

    bool ok = c11__stable_sort(self->data,
                               self->length,
                               sizeof(py_TValue),
                               (int (*)(const void*, const void*, void*))lt_with_key,
                               key);
    if(!ok) return false;

    PY_CHECK_ARG_TYPE(2, tp_bool);
    bool reverse = py_tobool(py_arg(2));
    if(reverse) c11__reverse(py_TValue, self);
    py_newnone(py_retval());
    return true;
}

static bool list__iter__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    list_iterator* ud = py_newobject(py_retval(), tp_list_iterator, 1, sizeof(list_iterator));
    ud->vec = py_touserdata(argv);
    ud->index = 0;
    py_setslot(py_retval(), 0, argv);  // keep a reference to the object
    return true;
}

static bool list__contains__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    return pk_arraycontains(py_arg(0), py_arg(1));
}

py_Type pk_list__register() {
    py_Type type =
        pk_newtype("list", tp_object, NULL, (void (*)(void*))c11_vector__dtor, false, true);

    py_bindmagic(type, __len__, list__len__);
    py_bindmagic(type, __eq__, list__eq__);
    py_bindmagic(type, __ne__, list__ne__);
    py_bindmagic(type, __new__, list__new__);
    py_bindmagic(type, __getitem__, list__getitem__);
    py_bindmagic(type, __setitem__, list__setitem__);
    py_bindmagic(type, __delitem__, list__delitem__);
    py_bindmagic(type, __add__, list__add__);
    py_bindmagic(type, __mul__, list__mul__);
    py_bindmagic(type, __rmul__, list__rmul__);
    py_bindmagic(type, __repr__, list__repr__);
    py_bindmagic(type, __iter__, list__iter__);
    py_bindmagic(type, __contains__, list__contains__);

    py_bindmethod(type, "append", list_append);
    py_bindmethod(type, "extend", list_extend);
    py_bindmethod(type, "count", list_count);
    py_bindmethod(type, "clear", list_clear);
    py_bindmethod(type, "copy", list_copy);
    py_bindmethod(type, "index", list_index);
    py_bindmethod(type, "reverse", list_reverse);
    py_bindmethod(type, "remove", list_remove);
    py_bindmethod(type, "pop", list_pop);
    py_bindmethod(type, "insert", list_insert);
    py_bindmethod(type, "sort", list_sort);

    py_bind(py_tpobject(type), "sort(self, key=None, reverse=False)", list_sort);

    py_setdict(py_tpobject(type), __hash__, py_None());
    return type;
}
