#include "pocketpy/interpreter/array2d.h"

static bool c11_array2d_like_is_valid(c11_array2d_like* self, unsigned int col, unsigned int row) {
    return col < self->n_cols && row < self->n_rows;
}

static py_Ref py_array2d__get(c11_array2d* self, int col, int row) {
    return self->data + row * self->header.n_cols + col;
}
static bool py_array2d__set(c11_array2d* self, int col, int row, py_Ref value) {
    self->data[row * self->header.n_cols + col] = *value;
    return true;
}

c11_array2d* py_newarray2d(py_OutRef out, int n_cols, int n_rows) {
    int numel = n_cols * n_rows;
    c11_array2d* ud = py_newobject(out, tp_array2d, numel, sizeof(c11_array2d));
    ud->header.n_cols = n_cols;
    ud->header.n_rows = n_rows;
    ud->header.numel = numel;
    ud->header.f_get = (py_Ref (*)(c11_array2d_like*, int, int))py_array2d__get;
    ud->header.f_set = (bool (*)(c11_array2d_like*, int, int, py_Ref))py_array2d__set;
    ud->data = py_getslot(out, 0);
    return ud;
}

/* array2d_like bindings */
static bool array2d_like_n_cols(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_array2d_like* self = py_touserdata(argv);
    py_newint(py_retval(), self->n_cols);
    return true;
}

static bool array2d_like_n_rows(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_array2d_like* self = py_touserdata(argv);
    py_newint(py_retval(), self->n_rows);
    return true;
}

static bool array2d_like_shape(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_array2d_like* self = py_touserdata(argv);
    c11_vec2i shape;
    shape.x = self->n_cols;
    shape.y = self->n_rows;
    py_newvec2i(py_retval(), shape);
    return true;
}

static bool array2d_like_numel(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_array2d_like* self = py_touserdata(argv);
    py_newint(py_retval(), self->numel);
    return true;
}

static bool array2d_like_is_valid(int argc, py_Ref argv) {
    c11_array2d_like* self = py_touserdata(argv);
    int col, row;
    if(argc == 2) {
        PY_CHECK_ARG_TYPE(1, tp_vec2i);
        c11_vec2i pos = py_tovec2i(py_arg(1));
        col = pos.x;
        row = pos.y;
    } else if(argc == 3) {
        PY_CHECK_ARG_TYPE(1, tp_int);
        PY_CHECK_ARG_TYPE(2, tp_int);
        col = py_toint(py_arg(1));
        row = py_toint(py_arg(2));
    } else {
        return TypeError("is_valid() expected 2 or 3 arguments");
    }
    py_newbool(py_retval(), c11_array2d_like_is_valid(self, col, row));
    return true;
}

static bool array2d_like_get(int argc, py_Ref argv) {
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    py_Ref default_;
    c11_array2d_like* self = py_touserdata(argv);
    if(argc == 3) {
        default_ = py_None();
    } else if(argc == 4) {
        default_ = py_arg(3);
    } else {
        return TypeError("get() expected 2 or 3 arguments");
    }
    int col = py_toint(py_arg(1));
    int row = py_toint(py_arg(2));
    if(c11_array2d_like_is_valid(self, col, row)) {
        py_assign(py_retval(), self->f_get(self, col, row));
    } else {
        py_assign(py_retval(), default_);
    }
    return true;
}

static bool array2d_like_render(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    c11_array2d_like* self = py_touserdata(argv);
    for(int j = 0; j < self->n_rows; j++) {
        for(int i = 0; i < self->n_cols; i++) {
            py_Ref item = self->f_get(self, i, j);
            if(!py_str(item)) return false;
            c11_sbuf__write_sv(&buf, py_tosv(py_retval()));
        }
        if(j < self->n_rows - 1) c11_sbuf__write_char(&buf, '\n');
    }
    c11_sbuf__py_submit(&buf, py_retval());
    return true;
}

static bool array2d_like_all(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_array2d_like* self = py_touserdata(argv);
    for(int j = 0; j < self->n_rows; j++) {
        for(int i = 0; i < self->n_cols; i++) {
            py_Ref item = self->f_get(self, i, j);
            if(!py_checkbool(item)) return false;
            if(!py_tobool(item)) {
                py_newbool(py_retval(), false);
                return true;
            }
        }
    }
    py_newbool(py_retval(), true);
    return true;
}

static bool array2d_like_any(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_array2d_like* self = py_touserdata(argv);
    for(int j = 0; j < self->n_rows; j++) {
        for(int i = 0; i < self->n_cols; i++) {
            py_Ref item = self->f_get(self, i, j);
            if(!py_checkbool(item)) return false;
            if(py_tobool(item)) {
                py_newbool(py_retval(), true);
                return true;
            }
        }
    }
    py_newbool(py_retval(), false);
    return true;
}

static bool array2d_like_map(int argc, py_Ref argv) {
    // def map(self, f: Callable[[T], Any]) -> 'array2d': ...
    PY_CHECK_ARGC(2);
    c11_array2d_like* self = py_touserdata(argv);
    py_Ref f = py_arg(1);
    c11_array2d* res = py_newarray2d(py_pushtmp(), self->n_cols, self->n_rows);
    for(int j = 0; j < self->n_rows; j++) {
        for(int i = 0; i < self->n_cols; i++) {
            py_Ref item = self->f_get(self, i, j);
            if(!py_call(f, 1, item)) return false;
            res->data[j * self->n_cols + i] = *py_retval();
        }
    }
    py_assign(py_retval(), py_peek(-1));
    py_pop();
    return true;
}

static bool array2d_like_copy(int argc, py_Ref argv) {
    // def copy(self) -> 'array2d': ...
    PY_CHECK_ARGC(1);
    c11_array2d_like* self = py_touserdata(argv);
    c11_array2d* res = py_newarray2d(py_retval(), self->n_cols, self->n_rows);
    for(int j = 0; j < self->n_rows; j++) {
        for(int i = 0; i < self->n_cols; i++) {
            py_Ref item = self->f_get(self, i, j);
            res->data[j * self->n_cols + i] = *item;
        }
    }
    return true;
}

static bool array2d_like_apply(int argc, py_Ref argv) {
    // def apply_(self, f: Callable[[T], T]) -> None: ...
    PY_CHECK_ARGC(2);
    c11_array2d_like* self = py_touserdata(argv);
    py_Ref f = py_arg(1);
    for(int j = 0; j < self->n_rows; j++) {
        for(int i = 0; i < self->n_cols; i++) {
            py_Ref item = self->f_get(self, i, j);
            if(!py_call(f, 1, item)) return false;
            bool ok = self->f_set(self, i, j, py_retval());
            if(!ok) return false;
        }
    }
    py_newnone(py_retval());
    return true;
}

static void pk__register_array2d_like(py_Ref mod) {
    py_Type type = py_newtype("array2d_like", tp_object, mod, NULL);

    py_bindproperty(type, "n_cols", array2d_like_n_cols, NULL);
    py_bindproperty(type, "n_rows", array2d_like_n_rows, NULL);
    py_bindproperty(type, "width", array2d_like_n_cols, NULL);
    py_bindproperty(type, "height", array2d_like_n_rows, NULL);
    py_bindproperty(type, "shape", array2d_like_shape, NULL);
    py_bindproperty(type, "numel", array2d_like_numel, NULL);

    py_bindmethod(type, "is_valid", array2d_like_is_valid);
    py_bindmethod(type, "get", array2d_like_get);

    py_bindmethod(type, "render", array2d_like_render);

    py_bindmethod(type, "all", array2d_like_all);
    py_bindmethod(type, "any", array2d_like_any);

    py_bindmethod(type, "map", array2d_like_map);
    py_bindmethod(type, "apply", array2d_like_apply);
    py_bindmethod(type, "copy", array2d_like_copy);
}

static bool array2d__new__(int argc, py_Ref argv) {
    // __new__(cls, n_cols: int, n_rows: int, default: Callable[[vec2i], T] = None)
    py_Ref default_ = py_arg(3);
    PY_CHECK_ARG_TYPE(0, tp_type);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    int n_cols = argv[1]._i64;
    int n_rows = argv[2]._i64;
    if(n_cols <= 0 || n_rows <= 0) return ValueError("array2d() expected positive dimensions");
    c11_array2d* ud = py_newarray2d(py_pushtmp(), n_cols, n_rows);
    // setup initial values
    if(py_callable(default_)) {
        for(int j = 0; j < n_rows; j++) {
            for(int i = 0; i < n_cols; i++) {
                py_TValue tmp;
                py_newvec2i(&tmp, (c11_vec2i){{i, j}});
                if(!py_call(default_, 1, &tmp)) return false;
                ud->data[j * n_cols + i] = *py_retval();
            }
        }
    } else {
        for(int i = 0; i < ud->header.numel; i++) {
            ud->data[i] = *default_;
        }
    }
    py_assign(py_retval(), py_peek(-1));
    py_pop();
    return true;
}



static bool _array2d_check_all_type(c11_array2d* self, py_Type type) {
    for(int i = 0; i < self->numel; i++) {
        py_Type item_type = self->data[i].type;
        if(item_type != type) {
            const char* fmt = "expected array2d[%t], got %t";
            return TypeError(fmt, type, item_type);
        }
    }
    return true;
}

static bool _check_same_shape(int colA, int rowA, int colB, int rowB) {
    if(colA != colB || rowA != rowB) {
        const char* fmt = "expected the same shape: (%d, %d) != (%d, %d)";
        return ValueError(fmt, colA, rowA, colB, rowB);
    }
    return true;
}

static bool _array2d_check_same_shape(c11_array2d* self, c11_array2d* other) {
    return _check_same_shape(self->n_cols, self->n_rows, other->n_cols, other->n_rows);
}



static bool array2d__eq__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_array2d* self = py_touserdata(argv);
    c11_array2d* res = py_newarray2d(py_pushtmp(), self->n_cols, self->n_rows);
    if(py_istype(py_arg(1), tp_array2d)) {
        c11_array2d* other = py_touserdata(py_arg(1));
        if(!_array2d_check_same_shape(self, other)) return false;
        for(int i = 0; i < self->numel; i++) {
            int code = py_equal(self->data + i, other->data + i);
            if(code == -1) return false;
            py_newbool(res->data + i, (bool)code);
        }
    } else {
        // broadcast
        for(int i = 0; i < self->numel; i++) {
            int code = py_equal(self->data + i, py_arg(1));
            if(code == -1) return false;
            py_newbool(res->data + i, (bool)code);
        }
    }
    py_assign(py_retval(), py_peek(-1));
    py_pop();
    return true;
}

static bool array2d__ne__(int argc, py_Ref argv) {
    bool ok = array2d__eq__(argc, argv);
    if(!ok) return false;
    c11_array2d* res = py_touserdata(py_retval());
    py_TValue* data = res->data;
    for(int i = 0; i < res->numel; i++) {
        py_newbool(&data[i], !py_tobool(&data[i]));
    }
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

// __iter__(self) -> Iterator[tuple[int, int, T]]
static bool array2d_iterator__next__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_array2d_iterator* self = py_touserdata(argv);
    if(self->index < self->array->numel) {
        div_t res = div(self->index, self->array->n_cols);
        py_newtuple(py_retval(), 2);
        py_TValue* data = py_tuple_data(py_retval());
        py_newvec2i(&data[0],
                    (c11_vec2i){
                        {res.rem, res.quot}
        });
        py_assign(&data[1], self->array->data + self->index);
        self->index++;
        return true;
    }
    return StopIteration();
}


// fromlist(data: list[list[T]]) -> array2d[T]
static bool array2d_fromlist_STATIC(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    if(!py_checktype(argv, tp_list)) return false;
    int n_rows = py_list_len(argv);
    if(n_rows == 0) return ValueError("fromlist() expected a non-empty list");
    int n_cols = -1;
    for(int j = 0; j < n_rows; j++) {
        py_Ref row_j = py_list_getitem(argv, j);
        if(!py_checktype(row_j, tp_list)) return false;
        int n_cols_j = py_list_len(row_j);
        if(n_cols == -1) {
            if(n_cols_j == 0) return ValueError("fromlist() expected a non-empty list");
            n_cols = n_cols_j;
        } else if(n_cols != n_cols_j) {
            return ValueError("fromlist() expected a list of lists with the same length");
        }
    }
    c11_array2d* res = py_newarray2d(py_retval(), n_cols, n_rows);
    for(int j = 0; j < n_rows; j++) {
        py_Ref row_j = py_list_getitem(argv, j);
        for(int i = 0; i < n_cols; i++) {
            py_array2d__set(res, i, j, py_list_getitem(row_j, i));
        }
    }
    return true;
}

// tolist(self) -> list[list[T]]
static bool array2d_tolist(int argc, py_Ref argv) {
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



// count(self, value: T) -> int
static bool array2d_count(int argc, py_Ref argv) {
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

// get_bounding_rect(self, value: T) -> tuple[int, int, int, int]
static bool array2d_get_bounding_rect(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
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
        return ValueError("value not found");
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

// count_neighbors(self, value: T, neighborhood: Neighborhood) -> array2d[int]
static bool array2d_count_neighbors(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    c11_array2d* self = py_touserdata(argv);
    c11_array2d* res = py_newarray2d(py_pushtmp(), self->n_cols, self->n_rows);
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

static bool _array2d_IndexError(c11_array2d* self, int col, int row) {
    return IndexError("(%d, %d) is not a valid index of array2d(%d, %d)",
                      col,
                      row,
                      self->n_cols,
                      self->n_rows);
}

static bool array2d__getitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_array2d* self = py_touserdata(argv);
    if(argv[1].type == tp_vec2i) {
        // fastpath for vec2i
        c11_vec2i pos = py_tovec2i(&argv[1]);
        if(py_array2d_is_valid(self, pos.x, pos.y)) {
            py_assign(py_retval(), py_array2d__get(self, pos.x, pos.y));
            return true;
        }
        return _array2d_IndexError(self, pos.x, pos.y);
    }

    if(argv[1].type == tp_array2d) {
        c11_array2d* mask = py_touserdata(&argv[1]);
        if(!_array2d_check_same_shape(self, mask)) return false;
        if(!_array2d_check_all_type(mask, tp_bool)) return false;
        py_newlist(py_retval());
        for(int i = 0; i < self->numel; i++) {
            if(py_tobool(mask->data + i)) py_list_append(py_retval(), self->data + i);
        }
        return true;
    }

    PY_CHECK_ARG_TYPE(1, tp_tuple);
    if(py_tuple_len(py_arg(1)) != 2) return TypeError("expected a tuple of 2 elements");
    py_Ref x = py_tuple_getitem(py_arg(1), 0);
    py_Ref y = py_tuple_getitem(py_arg(1), 1);
    if(py_isint(x) && py_isint(y)) {
        int col = py_toint(x);
        int row = py_toint(y);
        if(py_array2d_is_valid(self, col, row)) {
            py_assign(py_retval(), py_array2d__get(self, col, row));
            return true;
        }
        return _array2d_IndexError(self, col, row);
    } else if(py_istype(x, tp_slice) && py_istype(y, tp_slice)) {
        HANDLE_SLICE();
        c11_array2d* res = py_newarray2d(py_retval(), slice_width, slice_height);
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
    c11_array2d* self = py_touserdata(argv);
    py_Ref value = py_arg(2);
    if(argv[1].type == tp_vec2i) {
        // fastpath for vec2i
        c11_vec2i pos = py_tovec2i(&argv[1]);
        if(py_array2d_is_valid(self, pos.x, pos.y)) {
            py_array2d__set(self, pos.x, pos.y, value);
            py_newnone(py_retval());
            return true;
        }
        return _array2d_IndexError(self, pos.x, pos.y);
    }

    if(argv[1].type == tp_array2d) {
        c11_array2d* mask = py_touserdata(&argv[1]);
        if(!_array2d_check_same_shape(self, mask)) return false;
        if(!_array2d_check_all_type(mask, tp_bool)) return false;
        for(int i = 0; i < self->numel; i++) {
            if(py_tobool(mask->data + i)) self->data[i] = *value;
        }
        py_newnone(py_retval());
        return true;
    }

    PY_CHECK_ARG_TYPE(1, tp_tuple);
    if(py_tuple_len(py_arg(1)) != 2) return TypeError("expected a tuple of 2 elements");
    py_Ref x = py_tuple_getitem(py_arg(1), 0);
    py_Ref y = py_tuple_getitem(py_arg(1), 1);
    if(py_isint(x) && py_isint(y)) {
        int col = py_toint(x);
        int row = py_toint(y);
        if(py_array2d_is_valid(self, col, row)) {
            py_array2d__set(self, col, row, value);
            py_newnone(py_retval());
            return true;
        }
        return _array2d_IndexError(self, col, row);
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
            if(!_check_same_shape(slice_width, slice_height, src->n_cols, src->n_rows))
                return false;
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

// convolve(self: array2d[int], kernel: array2d[int], padding: int) -> array2d[int]
static bool array2d_convolve(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(1, tp_array2d);
    PY_CHECK_ARG_TYPE(2, tp_int);
    c11_array2d* self = py_touserdata(argv);
    c11_array2d* kernel = py_touserdata(py_arg(1));
    int padding = py_toint(py_arg(2));
    if(kernel->n_cols != kernel->n_rows) { return ValueError("kernel must be square"); }
    int ksize = kernel->n_cols;
    if(ksize % 2 == 0) return ValueError("kernel size must be odd");
    int ksize_half = ksize / 2;
    if(!_array2d_check_all_type(self, tp_int)) return false;
    if(!_array2d_check_all_type(kernel, tp_int)) return false;
    c11_array2d* res = py_newarray2d(py_pushtmp(), self->n_cols, self->n_rows);
    for(int j = 0; j < self->n_rows; j++) {
        for(int i = 0; i < self->n_cols; i++) {
            py_i64 sum = 0;
            for(int jj = 0; jj < ksize; jj++) {
                for(int ii = 0; ii < ksize; ii++) {
                    int x = i + ii - ksize_half;
                    int y = j + jj - ksize_half;
                    py_i64 _0, _1;
                    if(x < 0 || x >= self->n_cols || y < 0 || y >= self->n_rows) {
                        _0 = padding;
                    } else {
                        _0 = py_toint(py_array2d__get(self, x, y));
                    }
                    _1 = py_toint(py_array2d__get(kernel, ii, jj));
                    sum += _0 * _1;
                }
            }
            py_newint(py_array2d__get(res, i, j), sum);
        }
    }
    py_assign(py_retval(), py_peek(-1));
    py_pop();
    return true;
}

void pk__add_module_array2d() {
    py_GlobalRef mod = py_newmodule("array2d");
    py_Type array2d = pk_newtype("array2d", tp_object, mod, NULL, false, true);
    py_Type array2d_iterator = pk_newtype("array2d_iterator", tp_object, mod, NULL, false, true);
    assert(array2d == tp_array2d);
    assert(array2d_iterator == tp_array2d_iterator);

    py_setdict(mod, py_name("array2d"), py_tpobject(array2d));

    // array2d is unhashable
    py_setdict(py_tpobject(array2d), __hash__, py_None());

    py_bindmagic(array2d_iterator, __iter__, pk_wrapper__self);
    py_bindmagic(array2d_iterator, __next__, array2d_iterator__next__);
    py_bind(py_tpobject(array2d),
            "__new__(cls, n_cols: int, n_rows: int, default=None)",
            array2d__new__);

    py_bindmagic(array2d, __eq__, array2d__eq__);
    py_bindmagic(array2d, __ne__, array2d__ne__);
    py_bindmagic(array2d, __repr__, array2d__repr__);
    py_bindmagic(array2d, __iter__, array2d__iter__);

    py_bindmagic(array2d, __getitem__, array2d__getitem__);
    py_bindmagic(array2d, __setitem__, array2d__setitem__);

    py_bindmethod(array2d, "is_valid", array2d_is_valid);
    py_bindmethod(array2d, "get", array2d_get);

    py_bindmethod(array2d, "map", array2d_map);
    py_bindmethod(array2d, "copy", array2d_copy);

    py_bindmethod(array2d, "fill_", array2d_fill_);
    py_bindmethod(array2d, "apply_", array2d_apply_);
    py_bindmethod(array2d, "copy_", array2d_copy_);

    py_bindmethod(array2d, "render", array2d_render);

    py_bindmethod(array2d, "all", array2d_all);
    py_bindmethod(array2d, "any", array2d_any);

    py_bindstaticmethod(array2d, "fromlist", array2d_fromlist_STATIC);
    py_bindmethod(array2d, "tolist", array2d_tolist);

    py_bindmethod(array2d, "count", array2d_count);
    py_bindmethod(array2d, "get_bounding_rect", array2d_get_bounding_rect);
    py_bindmethod(array2d, "count_neighbors", array2d_count_neighbors);
    py_bindmethod(array2d, "convolve", array2d_convolve);

    const char* scc =
        "\ndef get_connected_components(self, value: T, neighborhood: Neighborhood) -> tuple[array2d[int], int]:\n    from collections import deque\n    from linalg import vec2i\n\n    DIRS = [vec2i.LEFT, vec2i.RIGHT, vec2i.UP, vec2i.DOWN]\n    assert neighborhood in ['Moore', 'von Neumann']\n\n    if neighborhood == 'Moore':\n        DIRS.extend([\n            vec2i.LEFT+vec2i.UP,\n            vec2i.RIGHT+vec2i.UP,\n            vec2i.LEFT+vec2i.DOWN,\n            vec2i.RIGHT+vec2i.DOWN\n            ])\n\n    visited = array2d[int](self.width, self.height, default=0)\n    queue = deque()\n    count = 0\n    for y in range(self.height):\n        for x in range(self.width):\n            if visited[x, y] or self[x, y] != value:\n                continue\n            count += 1\n            queue.append((x, y))\n            visited[x, y] = count\n            while queue:\n                cx, cy = queue.popleft()\n                for dx, dy in DIRS:\n                    nx, ny = cx+dx, cy+dy\n                    if self.is_valid(nx, ny) and not visited[nx, ny] and self[nx, ny] == value:\n                        queue.append((nx, ny))\n                        visited[nx, ny] = count\n    return visited, count\n\narray2d.get_connected_components = get_connected_components\ndel get_connected_components\n";

    if(!py_exec(scc, "array2d.py", EXEC_MODE, mod)) {
        py_printexc();
        c11__abort("failed to execute array2d.py");
    }

    pk__register_chunked_array2d(mod);
}

#undef INC_COUNT
#undef HANDLE_SLICE

/* chunked_array2d */
#define SMALLMAP_T__SOURCE
#define K c11_vec2i
#define V py_TValue*
#define NAME c11_chunked_array2d_chunks
#define less(a, b) (a._i64 < b._i64)
#define equal(a, b) (a._i64 == b._i64)
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__SOURCE

static py_TValue* c11_chunked_array2d__new_chunk(c11_chunked_array2d* self, c11_vec2i pos) {
#ifndef NDEBUG
    bool exists = c11_chunked_array2d_chunks__contains(&self->chunks, pos);
    assert(!exists);
#endif
    int chunk_numel = self->chunk_size * self->chunk_size + 1;
    py_TValue* data = PK_MALLOC(sizeof(py_TValue) * chunk_numel);
    if(!py_isnone(&self->context_builder)) {
        py_newvec2i(&data[0], pos);
        bool ok = py_call(&self->context_builder, 1, &data[0]);
        if(!ok) return NULL;
        data[0] = *py_retval();
    } else {
        data[0] = *py_None();
    }
    memset(&data[1], 0, sizeof(py_TValue) * (chunk_numel - 1));
    c11_chunked_array2d_chunks__set(&self->chunks, pos, data);
    self->last_visited.key = pos;
    self->last_visited.value = data;
    return data;
}

void c11_chunked_array2d__world_to_chunk(c11_chunked_array2d* self,
                                         int col,
                                         int row,
                                         c11_vec2i* chunk_pos,
                                         c11_vec2i* local_pos) {
    if(col >= 0) {
        chunk_pos->x = col >> self->chunk_size_log2;
        local_pos->x = col & self->chunk_size_mask;
    } else {
        chunk_pos->x = -((-col) >> self->chunk_size_log2);
        local_pos->x = (-col) & self->chunk_size_mask;
    }
    if(row >= 0) {
        chunk_pos->y = row >> self->chunk_size_log2;
        local_pos->y = row & self->chunk_size_mask;
    } else {
        chunk_pos->y = -((-row) >> self->chunk_size_log2);
        local_pos->y = (-row) & self->chunk_size_mask;
    }
}

static py_TValue* c11_chunked_array2d__parse_col_row(c11_chunked_array2d* self,
                                                     int col,
                                                     int row,
                                                     c11_vec2i* chunk_pos,
                                                     c11_vec2i* local_pos) {
    c11_chunked_array2d__world_to_chunk(self, col, row, chunk_pos, local_pos);
    py_TValue* data;
    if(self->last_visited.value != NULL && chunk_pos->_i64 == self->last_visited.key._i64) {
        data = self->last_visited.value;
    } else {
        data = c11_chunked_array2d_chunks__get(&self->chunks, *chunk_pos, NULL);
    }
    if(data != NULL) {
        self->last_visited.key = *chunk_pos;
        self->last_visited.value = data;
    }
    return data + 1;  // skip context
}

static bool chunked_array2d__new__(int argc, py_Ref argv) {
    py_Type cls = py_totype(argv);
    py_newobject(py_retval(), cls, 0, sizeof(c11_chunked_array2d));
    return true;
}

static bool chunked_array2d__init__(int argc, py_Ref argv) {
    c11_chunked_array2d* self = py_touserdata(&argv[0]);
    PY_CHECK_ARG_TYPE(1, tp_int);
    int chunk_size = py_toint(&argv[1]);
    self->default_T = argv[2];
    self->context_builder = argv[3];

    c11_chunked_array2d_chunks__ctor(&self->chunks);
    self->chunk_size = chunk_size;
    switch(chunk_size) {
        case 2: self->chunk_size_log2 = 1; break;
        case 4: self->chunk_size_log2 = 2; break;
        case 8: self->chunk_size_log2 = 3; break;
        case 16: self->chunk_size_log2 = 4; break;
        case 32: self->chunk_size_log2 = 5; break;
        case 64: self->chunk_size_log2 = 6; break;
        case 128: self->chunk_size_log2 = 7; break;
        case 256: self->chunk_size_log2 = 8; break;
        case 512: self->chunk_size_log2 = 9; break;
        case 1024: self->chunk_size_log2 = 10; break;
        case 2048: self->chunk_size_log2 = 11; break;
        case 4096: self->chunk_size_log2 = 12; break;
        default: return ValueError("invalid chunk_size: %d", chunk_size);
    }
    self->chunk_size_mask = chunk_size - 1;
    memset(&self->last_visited, 0, sizeof(c11_chunked_array2d_chunks_KV));
    py_newnone(py_retval());
    return true;
}

static bool chunked_array2d__chunk_size(int argc, py_Ref argv) {
    c11_chunked_array2d* self = py_touserdata(argv);
    py_newint(py_retval(), self->chunk_size);
    return true;
}

static bool chunked_array2d__getitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_vec2i);
    c11_chunked_array2d* self = py_touserdata(argv);
    c11_vec2i pos = py_tovec2i(&argv[1]);
    py_Ref res = c11_chunked_array2d__get(self, pos.x, pos.y);
    if(res != NULL) {
        py_assign(py_retval(), res);
    } else {
        py_assign(py_retval(), &self->default_T);
    }
    return true;
}

static bool chunked_array2d__setitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(1, tp_vec2i);
    c11_chunked_array2d* self = py_touserdata(argv);
    c11_vec2i pos = py_tovec2i(&argv[1]);
    bool ok = c11_chunked_array2d__set(self, pos.x, pos.y, &argv[2]);
    if(!ok) return false;
    py_newnone(py_retval());
    return true;
}

static bool chunked_array2d__delitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_vec2i);
    c11_chunked_array2d* self = py_touserdata(argv);
    c11_vec2i pos = py_tovec2i(&argv[1]);
    c11_chunked_array2d__del(self, pos.x, pos.y);
    py_newnone(py_retval());
    return true;
}

static bool chunked_array2d__iter__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_chunked_array2d* self = py_touserdata(argv);
    py_newtuple(py_retval(), self->chunks.length);
    for(int i = 0; i < self->chunks.length; i++) {
        py_TValue* data = c11__getitem(c11_chunked_array2d_chunks_KV, &self->chunks, i).value;
        py_tuple_setitem(py_retval(), i, &data[0]);
    }
    return py_iter(py_retval());
}

static bool chunked_array2d__clear(int argc, py_Ref argv) {
    c11_chunked_array2d* self = py_touserdata(argv);
    c11_chunked_array2d_chunks__clear(&self->chunks);
    self->last_visited.value = NULL;
    py_newnone(py_retval());
    return true;
}

static bool chunked_array2d__world_to_chunk(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_vec2);
    c11_chunked_array2d* self = py_touserdata(argv);
    c11_vec2i pos = py_tovec2i(&argv[1]);
    c11_vec2i chunk_pos, local_pos;
    c11_chunked_array2d__world_to_chunk(self, pos.x, pos.y, &chunk_pos, &local_pos);
    py_newtuple(py_retval(), 2);
    py_TValue* data = py_tuple_data(py_retval());
    py_newvec2i(&data[0], chunk_pos);
    py_newvec2i(&data[1], local_pos);
    return true;
}

static bool chunked_array2d__add_chunk(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_vec2i);
    c11_chunked_array2d* self = py_touserdata(argv);
    c11_vec2i pos = py_tovec2i(&argv[1]);
    py_TValue* data = c11_chunked_array2d__new_chunk(self, pos);
    if(data == NULL) return false;
    py_assign(py_retval(), &data[0]);  // context
    return true;
}

static bool chunked_array2d__remove_chunk(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_vec2i);
    c11_chunked_array2d* self = py_touserdata(argv);
    c11_vec2i pos = py_tovec2i(&argv[1]);
    bool ok = c11_chunked_array2d_chunks__del(&self->chunks, pos);
    py_newbool(py_retval(), ok);
    return true;
}

static bool chunked_array2d__get_context(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_vec2i);
    c11_chunked_array2d* self = py_touserdata(argv);
    c11_vec2i pos = py_tovec2i(&argv[1]);
    py_TValue* data = c11_chunked_array2d_chunks__get(&self->chunks, pos, NULL);
    if(data == NULL) {
        py_newnone(py_retval());
    } else {
        py_assign(py_retval(), &data[0]);
    }
    return true;
}

void c11_chunked_array2d__dtor(c11_chunked_array2d* self) {
    c11__foreach(c11_chunked_array2d_chunks_KV, &self->chunks, p_kv) PK_FREE(p_kv->value);
    c11_chunked_array2d_chunks__dtor(&self->chunks);
}

py_Ref c11_chunked_array2d__get(c11_chunked_array2d* self, int col, int row) {
    c11_vec2i chunk_pos, local_pos;
    py_TValue* data = c11_chunked_array2d__parse_col_row(self, col, row, &chunk_pos, &local_pos);
    if(data == NULL) return NULL;
    py_Ref retval = &data[local_pos.y * self->chunk_size + local_pos.x];
    if(py_isnil(retval)) return NULL;
    return retval;
}

bool c11_chunked_array2d__set(c11_chunked_array2d* self, int col, int row, py_Ref value) {
    c11_vec2i chunk_pos, local_pos;
    py_TValue* data = c11_chunked_array2d__parse_col_row(self, col, row, &chunk_pos, &local_pos);
    if(data == NULL) {
        data = c11_chunked_array2d__new_chunk(self, chunk_pos);
        if(data == NULL) return false;
    }
    data[local_pos.y * self->chunk_size + local_pos.x] = *value;
    return true;
}

void c11_chunked_array2d__del(c11_chunked_array2d* self, int col, int row) {
    c11_vec2i chunk_pos, local_pos;
    py_TValue* data = c11_chunked_array2d__parse_col_row(self, col, row, &chunk_pos, &local_pos);
    if(data != NULL) data[local_pos.y * self->chunk_size + local_pos.x] = *py_NIL();
}

static void c11_chunked_array2d__mark(void* ud) {
    c11_chunked_array2d* self = ud;
    pk__mark_value(&self->default_T);
    pk__mark_value(&self->context_builder);
    int chunk_numel = self->chunk_size * self->chunk_size + 1;
    for(int i = 0; i < self->chunks.length; i++) {
        py_TValue* data = c11__getitem(c11_chunked_array2d_chunks_KV, &self->chunks, i).value;
        for(int j = 0; j < chunk_numel; j++) {
            pk__mark_value(data + j);
        }
    }
}

void pk__register_chunked_array2d(py_Ref mod) {
    py_Type cls = py_newtype("chunked_array2d", tp_object, mod, (py_Dtor)c11_chunked_array2d__dtor);
    pk__tp_set_marker(cls, c11_chunked_array2d__mark);

    py_bindmagic(cls, __new__, chunked_array2d__new__);
    py_bind(py_tpobject(cls),
            "__init__(self, chunk_size, default=None, context_builder=None)",
            chunked_array2d__init__);

    py_bindproperty(cls, "chunk_size", chunked_array2d__chunk_size, NULL);

    py_bindmagic(cls, __getitem__, chunked_array2d__getitem__);
    py_bindmagic(cls, __setitem__, chunked_array2d__setitem__);
    py_bindmagic(cls, __delitem__, chunked_array2d__delitem__);
    py_bindmagic(cls, __iter__, chunked_array2d__iter__);

    py_bindmethod(cls, "clear", chunked_array2d__clear);
    py_bindmethod(cls, "world_to_chunk", chunked_array2d__world_to_chunk);
    py_bindmethod(cls, "add_chunk", chunked_array2d__add_chunk);
    py_bindmethod(cls, "remove_chunk", chunked_array2d__remove_chunk);
    py_bindmethod(cls, "get_context", chunked_array2d__get_context);
}