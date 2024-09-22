#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/vm.h"

typedef struct c11_array2d {
    py_TValue* data;  // slots
    int n_cols;
    int n_rows;
    int numel;
} c11_array2d;

typedef struct c11_array2d_iterator {
    c11_array2d* array;
    int index;
} c11_array2d_iterator;

static bool py_array2d_is_valid(c11_array2d* self, int col, int row) {
    return col >= 0 && col < self->n_cols && row >= 0 && row < self->n_rows;
}

static py_ObjectRef py_array2d__get(c11_array2d* self, int col, int row) {
    return self->data + row * self->n_cols + col;
}

static void py_array2d__set(c11_array2d* self, int col, int row, py_Ref value) {
    self->data[row * self->n_cols + col] = *value;
}

static c11_array2d* py_array2d(py_OutRef out, int n_cols, int n_rows) {
    int numel = n_cols * n_rows;
    c11_array2d* ud = py_newobject(out, tp_array2d, numel, sizeof(c11_array2d));
    ud->data = py_getslot(out, 0);
    ud->n_cols = n_cols;
    ud->n_rows = n_rows;
    ud->numel = numel;
    return ud;
}

/* bindings */
static bool array2d__new__(int argc, py_Ref argv) {
    // __new__(cls, n_cols: int, n_rows: int, default=None)
    py_Ref default_ = py_arg(3);
    PY_CHECK_ARG_TYPE(0, tp_type);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    int n_cols = argv[1]._i64;
    int n_rows = argv[2]._i64;
    int numel = n_cols * n_rows;
    if(n_cols <= 0 || n_rows <= 0) return ValueError("array2d() expected positive dimensions");
    c11_array2d* ud = py_array2d(py_pushtmp(), n_cols, n_rows);
    // setup initial values
    if(py_callable(default_)) {
        for(int i = 0; i < numel; i++) {
            bool ok = py_call(default_, 0, NULL);
            if(!ok) return false;
            ud->data[i] = *py_retval();
        }
    } else {
        for(int i = 0; i < numel; i++) {
            ud->data[i] = *default_;
        }
    }
    py_assign(py_retval(), py_peek(-1));
    py_pop();
    return true;
}

static bool array2d_n_cols(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_array2d* self = py_touserdata(argv);
    py_newint(py_retval(), self->n_cols);
    return true;
}

static bool array2d_n_rows(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_array2d* self = py_touserdata(argv);
    py_newint(py_retval(), self->n_rows);
    return true;
}

static bool array2d_numel(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_array2d* self = py_touserdata(argv);
    py_newint(py_retval(), self->numel);
    return true;
}

static bool array2d_is_valid(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    c11_array2d* self = py_touserdata(argv);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    int col = py_toint(py_arg(1));
    int row = py_toint(py_arg(2));
    py_newbool(py_retval(), py_array2d_is_valid(self, col, row));
    return true;
}

static bool array2d_get(int argc, py_Ref argv) {
    py_Ref default_;
    c11_array2d* self = py_touserdata(argv);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    if(argc == 3) {
        default_ = py_None();
    } else if(argc == 4) {
        default_ = py_arg(3);
    } else {
        return TypeError("get() expected 3 or 4 arguments");
    }
    int col = py_toint(py_arg(1));
    int row = py_toint(py_arg(2));
    if(py_array2d_is_valid(self, col, row)) {
        py_assign(py_retval(), py_array2d__get(self, col, row));
    } else {
        py_assign(py_retval(), default_);
    }
    return true;
}

static bool array2d_unsafe_get(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    c11_array2d* self = py_touserdata(argv);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    int col = py_toint(py_arg(1));
    int row = py_toint(py_arg(2));
    py_assign(py_retval(), py_array2d__get(self, col, row));
    return true;
}

static bool array2d_unsafe_set(int argc, py_Ref argv) {
    PY_CHECK_ARGC(4);
    c11_array2d* self = py_touserdata(argv);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    int col = py_toint(py_arg(1));
    int row = py_toint(py_arg(2));
    py_array2d__set(self, col, row, py_arg(3));
    py_newnone(py_retval());
    return true;
}

static bool array2d__len__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_array2d* self = py_touserdata(argv);
    py_newint(py_retval(), self->numel);
    return true;
}

static bool array2d__eq__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_array2d* self = py_touserdata(argv);
    if(!py_istype(py_arg(1), tp_array2d)) {
        py_newnotimplemented(py_retval());
        return true;
    }
    c11_array2d* other = py_touserdata(py_arg(1));
    if(self->n_cols != other->n_cols || self->n_rows != other->n_rows) {
        py_newbool(py_retval(), false);
        return true;
    }
    for(int i = 0; i < self->numel; i++) {
        int res = py_equal(self->data + i, other->data + i);
        if(res == -1) return false;
        if(res == 0) {
            py_newbool(py_retval(), false);
            return true;
        }
    }
    py_newbool(py_retval(), true);
    return true;
}

static bool array2d__ne__(int argc, py_Ref argv) {
    bool ok = array2d__eq__(argc, argv);
    if(!ok) return false;
    if(py_isbool(py_retval())) { py_newbool(py_retval(), !py_tobool(py_retval())); }
    return true;
}

static bool array2d__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_array2d* self = py_touserdata(argv);
    char buf[256];
    snprintf(buf, sizeof(buf), "array2d(%d, %d)", self->n_cols, self->n_rows);
    py_newstr(py_retval(), buf);
    return true;
}

static bool array2d__iter__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_array2d* self = py_touserdata(argv);
    c11_array2d_iterator* ud =
        py_newobject(py_retval(), tp_array2d_iterator, 1, sizeof(c11_array2d_iterator));
    py_setslot(py_retval(), 0, argv);  // keep the array alive
    ud->array = self;
    ud->index = 0;
    return true;
}

static bool array2d_iterator__next__(int argc, py_Ref argv) {
    // def __iter__(self) -> Iterator[tuple[int, int, T]]: ...
    PY_CHECK_ARGC(1);
    c11_array2d_iterator* self = py_touserdata(argv);
    if(self->index < self->array->numel) {
        div_t res = div(self->index, self->array->n_cols);
        py_newtuple(py_retval(), 3);
        py_TValue* data = py_tuple_data(py_retval());
        py_newint(&data[0], res.rem);
        py_newint(&data[1], res.quot);
        py_assign(&data[2], self->array->data + self->index);
        self->index++;
        return true;
    }
    return StopIteration();
}

static bool array2d_map(int argc, py_Ref argv) {
    // def map(self, f: Callable[[T], Any]) -> 'array2d': ...
    PY_CHECK_ARGC(2);
    c11_array2d* self = py_touserdata(argv);
    py_Ref f = py_arg(1);
    c11_array2d* res = py_array2d(py_pushtmp(), self->n_cols, self->n_rows);
    for(int i = 0; i < self->numel; i++) {
        bool ok = py_call(f, 1, self->data + i);
        if(!ok) return false;
        res->data[i] = *py_retval();
    }
    py_assign(py_retval(), py_peek(-1));
    py_pop();
    return true;
}

static bool array2d_copy(int argc, py_Ref argv) {
    // def copy(self) -> 'array2d': ...
    PY_CHECK_ARGC(1);
    c11_array2d* self = py_touserdata(argv);
    c11_array2d* res = py_array2d(py_retval(), self->n_cols, self->n_rows);
    memcpy(res->data, self->data, self->numel * sizeof(py_TValue));
    return true;
}

static bool array2d_fill_(int argc, py_Ref argv) {
    // def fill_(self, value: T) -> None: ...
    PY_CHECK_ARGC(2);
    c11_array2d* self = py_touserdata(argv);
    for(int i = 0; i < self->numel; i++)
        self->data[i] = argv[1];
    py_newnone(py_retval());
    return true;
}

static bool array2d_apply_(int argc, py_Ref argv) {
    // def apply_(self, f: Callable[[T], T]) -> None: ...
    PY_CHECK_ARGC(2);
    c11_array2d* self = py_touserdata(argv);
    py_Ref f = py_arg(1);
    for(int i = 0; i < self->numel; i++) {
        bool ok = py_call(f, 1, self->data + i);
        if(!ok) return false;
        self->data[i] = *py_retval();
    }
    py_newnone(py_retval());
    return true;
}

static bool array2d_copy_(int argc, py_Ref argv) {
    // def copy_(self, src: 'array2d') -> None: ...
    PY_CHECK_ARGC(2);
    c11_array2d* self = py_touserdata(argv);

    py_Type src_type = py_typeof(py_arg(1));
    if(src_type == tp_array2d) {
        c11_array2d* src = py_touserdata(py_arg(1));
        if(self->n_cols != src->n_cols || self->n_rows != src->n_rows) {
            return ValueError("copy_() expected the same shape: (%d, %d) != (%d, %d)",
                              self->n_cols,
                              self->n_rows,
                              src->n_cols,
                              src->n_rows);
        }
        memcpy(self->data, src->data, self->numel * sizeof(py_TValue));
    } else {
        py_TValue* data;
        int length = pk_arrayview(py_arg(1), &data);
        if(length != -1) {
            if(self->numel != length) {
                return ValueError("copy_() expected the same numel: %d != %d", self->numel, length);
            }
            memcpy(self->data, data, self->numel * sizeof(py_TValue));
        } else {
            return TypeError("copy_() expected `array2d`, `list` or `tuple`, got '%t", src_type);
        }
    }
    py_newnone(py_retval());
    return true;
}

static bool array2d_tolist(int argc, py_Ref argv) {
    // def tolist(self) -> list[list[T]]: ...
    PY_CHECK_ARGC(1);
    c11_array2d* self = py_touserdata(argv);
    py_newlistn(py_retval(), self->n_rows);
    for(int j = 0; j < self->n_rows; j++) {
        py_Ref row_j = py_list_getitem(py_retval(), j);
        py_newlistn(row_j, self->n_cols);
        for(int i = 0; i < self->n_cols; i++) {
            py_list_setitem(row_j, i, py_array2d__get(self, i, j));
        }
    }
    return true;
}

static bool array2d_count(int argc, py_Ref argv) {
    // def count(self, value: T) -> int: ...
    PY_CHECK_ARGC(2);
    c11_array2d* self = py_touserdata(argv);
    int count = 0;
    for(int i = 0; i < self->numel; i++) {
        int res = py_equal(self->data + i, &argv[1]);
        if(res == -1) return false;
        count += res;
    }
    py_newint(py_retval(), count);
    return true;
}

static bool array2d_find_bounding_rect(int argc, py_Ref argv) {
    c11_array2d* self = py_touserdata(argv);
    py_Ref value = py_arg(1);
    int left = self->n_cols;
    int top = self->n_rows;
    int right = 0;
    int bottom = 0;
    for(int j = 0; j < self->n_rows; j++) {
        for(int i = 0; i < self->n_cols; i++) {
            int res = py_equal(py_array2d__get(self, i, j), value);
            if(res == -1) return false;
            if(res == 1) {
                left = c11__min(left, i);
                top = c11__min(top, j);
                right = c11__max(right, i);
                bottom = c11__max(bottom, j);
            }
        }
    }
    int width = right - left + 1;
    int height = bottom - top + 1;
    if(width <= 0 || height <= 0) {
        py_newnone(py_retval());
    } else {
        py_newtuple(py_retval(), 4);
        py_TValue* data = py_tuple_data(py_retval());
        py_newint(&data[0], left);
        py_newint(&data[1], top);
        py_newint(&data[2], width);
        py_newint(&data[3], height);
    }
    return true;
}

static bool array2d_count_neighbors(int argc, py_Ref argv) {
    // def count_neighbors(self, value: T, neighborhood: Neighborhood) -> 'array2d[int]': ...
    PY_CHECK_ARGC(3);
    c11_array2d* self = py_touserdata(argv);
    c11_array2d* res = py_array2d(py_pushtmp(), self->n_cols, self->n_rows);
    py_Ref value = py_arg(1);
    const char* neighborhood = py_tostr(py_arg(2));

#define INC_COUNT(i, j)                                                                            \
    do {                                                                                           \
        if(py_array2d_is_valid(self, i, j)) {                                                      \
            int res = py_equal(py_array2d__get(self, i, j), value);                                \
            if(res == -1) return false;                                                            \
            count += res;                                                                          \
        }                                                                                          \
    } while(0)

    if(strcmp(neighborhood, "Moore") == 0) {
        for(int j = 0; j < res->n_rows; j++) {
            for(int i = 0; i < res->n_cols; i++) {
                int count = 0;
                INC_COUNT(i - 1, j - 1);
                INC_COUNT(i, j - 1);
                INC_COUNT(i + 1, j - 1);
                INC_COUNT(i - 1, j);
                INC_COUNT(i + 1, j);
                INC_COUNT(i - 1, j + 1);
                INC_COUNT(i, j + 1);
                INC_COUNT(i + 1, j + 1);
                py_newint(py_array2d__get(res, i, j), count);
            }
        }
    } else if(strcmp(neighborhood, "von Neumann") == 0) {
        for(int j = 0; j < res->n_rows; j++) {
            for(int i = 0; i < res->n_cols; i++) {
                int count = 0;
                INC_COUNT(i, j - 1);
                INC_COUNT(i - 1, j);
                INC_COUNT(i + 1, j);
                INC_COUNT(i, j + 1);
                py_newint(py_array2d__get(res, i, j), count);
            }
        }
    } else {
        return ValueError("neighborhood must be 'Moore' or 'von Neumann'");
    }
    py_assign(py_retval(), py_peek(-1));
    py_pop();
    return true;
}

#define HANDLE_SLICE()                                                                             \
    int start_col, stop_col, step_col;                                                             \
    int start_row, stop_row, step_row;                                                             \
    if(!pk__parse_int_slice(x, self->n_cols, &start_col, &stop_col, &step_col)) return false;      \
    if(!pk__parse_int_slice(y, self->n_rows, &start_row, &stop_row, &step_row)) return false;      \
    if(step_col != 1 || step_row != 1) return ValueError("slice step must be 1");                  \
    int slice_width = stop_col - start_col;                                                        \
    int slice_height = stop_row - start_row;                                                       \
    if(slice_width <= 0 || slice_height <= 0)                                                      \
        return ValueError("slice width and height must be positive");

static bool array2d__getitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_tuple);
    if(py_tuple_len(py_arg(1)) != 2) return TypeError("expected a tuple of 2 elements");
    py_Ref x = py_tuple_getitem(py_arg(1), 0);
    py_Ref y = py_tuple_getitem(py_arg(1), 1);
    c11_array2d* self = py_touserdata(argv);
    if(py_isint(x) && py_isint(y)) {
        int col = py_toint(x);
        int row = py_toint(y);
        if(py_array2d_is_valid(self, col, row)) {
            py_assign(py_retval(), py_array2d__get(self, col, row));
            return true;
        }
        return IndexError("(%d, %d) is not a valid index of array2d(%d, %d)",
                          col,
                          row,
                          self->n_cols,
                          self->n_rows);
    } else if(py_istype(x, tp_slice) && py_istype(y, tp_slice)) {
        HANDLE_SLICE();
        c11_array2d* res = py_array2d(py_retval(), slice_width, slice_height);
        for(int j = start_row; j < stop_row; j++) {
            for(int i = start_col; i < stop_col; i++) {
                py_array2d__set(res, i - start_col, j - start_row, py_array2d__get(self, i, j));
            }
        }
        return true;
    } else {
        return TypeError("expected a tuple of 2 ints or slices");
    }
}

static bool array2d__setitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(1, tp_tuple);
    if(py_tuple_len(py_arg(1)) != 2) return TypeError("expected a tuple of 2 elements");
    py_Ref x = py_tuple_getitem(py_arg(1), 0);
    py_Ref y = py_tuple_getitem(py_arg(1), 1);
    c11_array2d* self = py_touserdata(argv);
    py_Ref value = py_arg(2);
    if(py_isint(x) && py_isint(y)) {
        int col = py_toint(x);
        int row = py_toint(y);
        if(py_array2d_is_valid(self, col, row)) {
            py_array2d__set(self, col, row, value);
            py_newnone(py_retval());
            return true;
        }
        return IndexError("(%d, %d) is not a valid index of array2d(%d, %d)",
                          col,
                          row,
                          self->n_cols,
                          self->n_rows);
    } else if(py_istype(x, tp_slice) && py_istype(y, tp_slice)) {
        HANDLE_SLICE();
        bool is_basic_type = false;
        switch(value->type) {
            case tp_int:
            case tp_float:
            case tp_str:
            case tp_NoneType:
            case tp_bool: is_basic_type = true; break;
            default: {
                if(!py_istype(value, tp_array2d)) {
                    return TypeError("expected int/float/str/bool/None or an array2d instance");
                }
            }
        }

        if(is_basic_type) {
            for(int j = start_row; j < stop_row; j++) {
                for(int i = start_col; i < stop_col; i++) {
                    py_array2d__set(self, i, j, py_arg(2));
                }
            }
        } else {
            c11_array2d* src = py_touserdata(value);
            if(slice_width != src->n_cols || slice_height != src->n_rows) {
                return ValueError("expected the same shape: (%d, %d) != (%d, %d)",
                                  slice_width,
                                  slice_height,
                                  src->n_cols,
                                  src->n_rows);
            }
            for(int j = 0; j < slice_height; j++) {
                for(int i = 0; i < slice_width; i++) {
                    py_array2d__set(self, i + start_col, j + start_row, py_array2d__get(src, i, j));
                }
            }
        }
        py_newnone(py_retval());
        return true;
    } else {
        return TypeError("expected a tuple of 2 ints or slices");
    }
}

void pk__add_module_array2d() {
    py_GlobalRef mod = py_newmodule("array2d");
    py_Type array2d = pk_newtype("array2d", tp_object, mod, NULL, false, true);
    py_Type array2d_iterator = pk_newtype("array2d_iterator", tp_object, mod, NULL, false, true);
    assert(array2d == tp_array2d);
    assert(array2d_iterator == tp_array2d_iterator);

    py_setdict(mod, py_name("array2d"), py_tpobject(array2d));

    py_bindmagic(array2d_iterator, __iter__, pk_wrapper__self);
    py_bindmagic(array2d_iterator, __next__, array2d_iterator__next__);
    py_bind(py_tpobject(array2d),
            "__new__(cls, n_cols: int, n_rows: int, default=None)",
            array2d__new__);

    py_bindmagic(array2d, __len__, array2d__len__);
    py_bindmagic(array2d, __eq__, array2d__eq__);
    py_bindmagic(array2d, __ne__, array2d__ne__);
    py_bindmagic(array2d, __repr__, array2d__repr__);
    py_bindmagic(array2d, __iter__, array2d__iter__);

    py_bindmagic(array2d, __getitem__, array2d__getitem__);
    py_bindmagic(array2d, __setitem__, array2d__setitem__);

    py_bindproperty(array2d, "n_cols", array2d_n_cols, NULL);
    py_bindproperty(array2d, "n_rows", array2d_n_rows, NULL);
    py_bindproperty(array2d, "width", array2d_n_cols, NULL);
    py_bindproperty(array2d, "height", array2d_n_rows, NULL);
    py_bindproperty(array2d, "numel", array2d_numel, NULL);

    py_bindmethod(array2d, "is_valid", array2d_is_valid);
    py_bindmethod(array2d, "get", array2d_get);
    py_bindmethod(array2d, "unsafe_get", array2d_unsafe_get);
    py_bindmethod(array2d, "unsafe_set", array2d_unsafe_set);

    py_bindmethod(array2d, "map", array2d_map);
    py_bindmethod(array2d, "copy", array2d_copy);

    py_bindmethod(array2d, "fill_", array2d_fill_);
    py_bindmethod(array2d, "apply_", array2d_apply_);
    py_bindmethod(array2d, "copy_", array2d_copy_);

    py_bindmethod(array2d, "tolist", array2d_tolist);
    py_bindmethod(array2d, "count", array2d_count);
    py_bindmethod(array2d, "find_bounding_rect", array2d_find_bounding_rect);
    py_bindmethod(array2d, "count_neighbors", array2d_count_neighbors);
}