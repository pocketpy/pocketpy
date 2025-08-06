#include "pocketpy/interpreter/array2d.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/pocketpy.h"
#include <limits.h>

static bool c11_array2d_like_is_valid(c11_array2d_like* self, int col, int row) {
    return col >= 0 && col < self->n_cols && row >= 0 && row < self->n_rows;
}

static py_Ref c11_array2d__get(c11_array2d* self, int col, int row) {
    return self->data + row * self->header.n_cols + col;
}

static bool c11_array2d__set(c11_array2d* self, int col, int row, py_Ref value) {
    self->data[row * self->header.n_cols + col] = *value;
    return true;
}

c11_array2d* c11_newarray2d(py_OutRef out, int n_cols, int n_rows) {
    int numel = n_cols * n_rows;
    c11_array2d* ud = py_newobject(out, tp_array2d, numel, sizeof(c11_array2d));
    ud->header.n_cols = n_cols;
    ud->header.n_rows = n_rows;
    ud->header.numel = numel;
    ud->header.f_get = (py_Ref(*)(c11_array2d_like*, int, int))c11_array2d__get;
    ud->header.f_set = (bool (*)(c11_array2d_like*, int, int, py_Ref))c11_array2d__set;
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

static bool array2d_like_index(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_array2d_like* self = py_touserdata(argv);
    py_Ref value = py_arg(1);
    for(int j = 0; j < self->n_rows; j++) {
        for(int i = 0; i < self->n_cols; i++) {
            py_Ref item = self->f_get(self, i, j);
            int code = py_equal(item, value);
            if(code == -1) return false;
            if(code == 1) {
                py_newvec2i(py_retval(),
                            (c11_vec2i){
                                {i, j}
                });
                return true;
            }
        }
    }
    return ValueError("value not found");
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
    c11_array2d* res = c11_newarray2d(py_pushtmp(), self->n_cols, self->n_rows);
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

static bool _check_same_shape(int colA, int rowA, int colB, int rowB) {
    if(colA != colB || rowA != rowB) {
        const char* fmt = "expected the same shape: (%d, %d) != (%d, %d)";
        return ValueError(fmt, colA, rowA, colB, rowB);
    }
    return true;
}

static bool _array2d_like_check_same_shape(c11_array2d_like* self, c11_array2d_like* other) {
    return _check_same_shape(self->n_cols, self->n_rows, other->n_cols, other->n_rows);
}

static bool _array2d_like_broadcasted_zip_with(int argc, py_Ref argv, py_Name op, py_Name rop) {
    PY_CHECK_ARGC(2);
    c11_array2d_like* self = py_touserdata(argv);
    c11_array2d_like* other;
    if(py_isinstance(py_arg(1), tp_array2d_like)) {
        other = py_touserdata(py_arg(1));
        if(!_array2d_like_check_same_shape(self, other)) return false;
    } else {
        other = NULL;
    }
    c11_array2d* res = c11_newarray2d(py_pushtmp(), self->n_cols, self->n_rows);
    for(int j = 0; j < self->n_rows; j++) {
        for(int i = 0; i < self->n_cols; i++) {
            py_Ref lhs = self->f_get(self, i, j);
            py_Ref rhs;
            if(other != NULL) {
                rhs = other->f_get(other, i, j);
            } else {
                rhs = py_arg(1);  // broadcast
            }
            if(!py_binaryop(lhs, rhs, op, rop)) return false;
            c11_array2d__set(res, i, j, py_retval());
        }
    }
    py_assign(py_retval(), py_peek(-1));
    py_pop();
    return true;
}

static bool array2d_like_zip_with(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    c11_array2d_like* self = py_touserdata(argv);
    if(!py_checkinstance(py_arg(1), tp_array2d_like)) return false;
    c11_array2d_like* other = py_touserdata(py_arg(1));
    py_Ref f = py_arg(2);
    if(!_array2d_like_check_same_shape(self, other)) return false;
    c11_array2d* res = c11_newarray2d(py_pushtmp(), self->n_cols, self->n_rows);
    for(int j = 0; j < self->n_rows; j++) {
        for(int i = 0; i < self->n_cols; i++) {
            py_push(f);
            py_pushnil();
            py_push(self->f_get(self, i, j));
            py_push(other->f_get(other, i, j));
            if(!py_vectorcall(2, 0)) return false;
            c11_array2d__set(res, i, j, py_retval());
        }
    }
    py_assign(py_retval(), py_peek(-1));
    py_pop();
    return true;
}

#define DEF_ARRAY2D_LIKE__MAGIC_ZIP_WITH(name, op, rop)                                            \
    static bool array2d_like##name(int argc, py_Ref argv) {                                        \
        return _array2d_like_broadcasted_zip_with(argc, argv, op, rop);                            \
    }

DEF_ARRAY2D_LIKE__MAGIC_ZIP_WITH(__le__, __le__, __ge__)
DEF_ARRAY2D_LIKE__MAGIC_ZIP_WITH(__lt__, __lt__, __gt__)
DEF_ARRAY2D_LIKE__MAGIC_ZIP_WITH(__ge__, __ge__, __le__)
DEF_ARRAY2D_LIKE__MAGIC_ZIP_WITH(__gt__, __gt__, __lt__)
DEF_ARRAY2D_LIKE__MAGIC_ZIP_WITH(__eq__, __eq__, __eq__)
DEF_ARRAY2D_LIKE__MAGIC_ZIP_WITH(__ne__, __ne__, __ne__)

DEF_ARRAY2D_LIKE__MAGIC_ZIP_WITH(__add__, __add__, __radd__)
DEF_ARRAY2D_LIKE__MAGIC_ZIP_WITH(__sub__, __sub__, __rsub__)
DEF_ARRAY2D_LIKE__MAGIC_ZIP_WITH(__mul__, __mul__, __rmul__)
DEF_ARRAY2D_LIKE__MAGIC_ZIP_WITH(__truediv__, __truediv__, __rtruediv__)
DEF_ARRAY2D_LIKE__MAGIC_ZIP_WITH(__floordiv__, __floordiv__, __rfloordiv__)
DEF_ARRAY2D_LIKE__MAGIC_ZIP_WITH(__mod__, __mod__, __rmod__)
DEF_ARRAY2D_LIKE__MAGIC_ZIP_WITH(__pow__, __pow__, __rpow__)

DEF_ARRAY2D_LIKE__MAGIC_ZIP_WITH(__and__, __and__, 0)
DEF_ARRAY2D_LIKE__MAGIC_ZIP_WITH(__or__, __or__, 0)
DEF_ARRAY2D_LIKE__MAGIC_ZIP_WITH(__xor__, __xor__, 0)

#undef DEF_ARRAY2D_LIKE__MAGIC_ZIP_WITH

static bool array2d_like__invert__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_array2d_like* self = py_touserdata(argv);
    c11_array2d* res = c11_newarray2d(py_pushtmp(), self->n_cols, self->n_rows);
    for(int j = 0; j < self->n_rows; j++) {
        for(int i = 0; i < self->n_cols; i++) {
            py_Ref item = self->f_get(self, i, j);
            if(!pk_callmagic(__invert__, 1, item)) return false;
            c11_array2d__set(res, i, j, py_retval());
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
    c11_array2d* res = c11_newarray2d(py_retval(), self->n_cols, self->n_rows);
    for(int j = 0; j < self->n_rows; j++) {
        for(int i = 0; i < self->n_cols; i++) {
            py_Ref item = self->f_get(self, i, j);
            res->data[j * self->n_cols + i] = *item;
        }
    }
    return true;
}

static bool array2d_like_tolist(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_array2d_like* self = py_touserdata(argv);
    py_newlistn(py_retval(), self->n_rows);
    for(int j = 0; j < self->n_rows; j++) {
        py_Ref row_j = py_list_getitem(py_retval(), j);
        py_newlistn(row_j, self->n_cols);
        for(int i = 0; i < self->n_cols; i++) {
            py_Ref item = self->f_get(self, i, j);
            py_list_setitem(row_j, i, item);
        }
    }
    return true;
}

static bool array2d_like__iter__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_array2d_like* self = py_touserdata(argv);
    c11_array2d_like_iterator* ud =
        py_newobject(py_retval(), tp_array2d_like_iterator, 1, sizeof(c11_array2d_like_iterator));
    py_setslot(py_retval(), 0, argv);  // keep the array alive
    ud->array = self;
    ud->j = 0;
    ud->i = 0;
    return true;
}

static bool array2d_like__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_array2d_like* self = py_touserdata(argv);
    char buf[256];
    snprintf(buf,
             sizeof(buf),
             "%s(%d, %d)",
             py_tpname(py_typeof(argv)),
             self->n_cols,
             self->n_rows);
    py_newstr(py_retval(), buf);
    return true;
}

#define HANDLE_SLICE()                                                                             \
    int start_col, stop_col, step_col;                                                             \
    int start_row, stop_row, step_row;                                                             \
    if(!pk__parse_int_slice(x, self->n_cols, &start_col, &stop_col, &step_col)) return false;      \
    if(!pk__parse_int_slice(y, self->n_rows, &start_row, &stop_row, &step_row)) return false;      \
    if(step_col != 1 || step_row != 1) return ValueError("slice step must be 1");                  \
    int slice_width = stop_col - start_col;                                                        \
    int slice_height = stop_row - start_row;

static bool _array2d_like_IndexError(c11_array2d_like* self, int col, int row) {
    return IndexError("(%d, %d) is not a valid index of array2d_like(%d, %d)",
                      col,
                      row,
                      self->n_cols,
                      self->n_rows);
}

static py_Ref c11_array2d_view__get(c11_array2d_view* self, int col, int row) {
    return self->f_get(self->ctx, col + self->origin.x, row + self->origin.y);
}

static bool c11_array2d_view__set(c11_array2d_view* self, int col, int row, py_Ref value) {
    return self->f_set(self->ctx, col + self->origin.x, row + self->origin.y, value);
}

static c11_array2d_view* _array2d_view__new(py_OutRef out,
                                            py_Ref keepalive,
                                            int start_col,
                                            int start_row,
                                            int width,
                                            int height) {
    c11_array2d_view* res = py_newobject(out, tp_array2d_view, 1, sizeof(c11_array2d_view));
    if(width <= 0 || height <= 0) {
        ValueError("width and height must be positive");
        return NULL;
    }
    res->header.n_cols = width;
    res->header.n_rows = height;
    res->header.numel = width * height;
    res->header.f_get = (py_Ref(*)(c11_array2d_like*, int, int))c11_array2d_view__get;
    res->header.f_set = (bool (*)(c11_array2d_like*, int, int, py_Ref))c11_array2d_view__set;
    res->origin.x = start_col;
    res->origin.y = start_row;
    py_setslot(out, 0, keepalive);
    return res;
}

static bool _array2d_view(py_OutRef out,
                          py_Ref keepalive,
                          c11_array2d_like* array,
                          int start_col,
                          int start_row,
                          int width,
                          int height) {
    c11_array2d_view* res = _array2d_view__new(out, keepalive, start_col, start_row, width, height);
    if(res == NULL) return false;
    res->ctx = array;
    res->f_get = (py_Ref(*)(void*, int, int))array->f_get;
    res->f_set = (bool (*)(void*, int, int, py_Ref))array->f_set;
    return true;
}

static bool _chunked_array2d_view(py_OutRef out,
                                  py_Ref keepalive,
                                  c11_chunked_array2d* array,
                                  int start_col,
                                  int start_row,
                                  int width,
                                  int height) {
    c11_array2d_view* res = _array2d_view__new(out, keepalive, start_col, start_row, width, height);
    if(res == NULL) return false;
    res->ctx = array;
    res->f_get = (py_Ref(*)(void*, int, int))c11_chunked_array2d__get;
    res->f_set = (bool (*)(void*, int, int, py_Ref))c11_chunked_array2d__set;
    return true;
}

static bool array2d_like__getitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_array2d_like* self = py_touserdata(argv);
    if(argv[1].type == tp_vec2i) {
        c11_vec2i pos = py_tovec2i(&argv[1]);
        if(c11_array2d_like_is_valid(self, pos.x, pos.y)) {
            py_assign(py_retval(), self->f_get(self, pos.x, pos.y));
            return true;
        }
        return _array2d_like_IndexError(self, pos.x, pos.y);
    }

    if(py_isinstance(&argv[1], tp_array2d_like)) {
        c11_array2d_like* mask = py_touserdata(&argv[1]);
        if(!_array2d_like_check_same_shape(self, mask)) return false;
        py_newlist(py_retval());
        for(int j = 0; j < self->n_rows; j++) {
            for(int i = 0; i < self->n_cols; i++) {
                py_Ref item = self->f_get(self, i, j);
                py_Ref cond = mask->f_get(mask, i, j);
                if(!py_checkbool(cond)) return false;
                if(py_tobool(cond)) py_list_append(py_retval(), item);
            }
        }
        return true;
    }

    PY_CHECK_ARG_TYPE(1, tp_tuple);
    if(py_tuple_len(&argv[1]) != 2) return TypeError("expected a tuple of 2 elements");
    py_Ref x = py_tuple_getitem(&argv[1], 0);
    py_Ref y = py_tuple_getitem(&argv[1], 1);
    if(py_isint(x) && py_isint(y)) {
        int col = py_toint(x);
        int row = py_toint(y);
        if(c11_array2d_like_is_valid(self, col, row)) {
            py_assign(py_retval(), self->f_get(self, col, row));
            return true;
        }
        return _array2d_like_IndexError(self, col, row);
    }

    bool _1 = py_istype(x, tp_slice) && py_istype(y, tp_slice);
    bool _2 = py_istype(x, tp_int) && py_istype(y, tp_slice);
    bool _3 = py_istype(x, tp_slice) && py_istype(y, tp_int);
    if(_1 || _2 || _3) {
        HANDLE_SLICE();
        return _array2d_view(py_retval(),
                             argv,
                             self,
                             start_col,
                             start_row,
                             slice_width,
                             slice_height);
    }

    return TypeError("expected tuple[int, int] or tuple[slice, slice]");
}

static bool array2d_like__setitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    c11_array2d_like* self = py_touserdata(argv);
    py_Ref value = &argv[2];
    if(argv[1].type == tp_vec2i) {
        c11_vec2i pos = py_tovec2i(&argv[1]);
        if(c11_array2d_like_is_valid(self, pos.x, pos.y)) {
            bool ok = self->f_set(self, pos.x, pos.y, value);
            if(!ok) return false;
            py_newnone(py_retval());
            return true;
        }
        return _array2d_like_IndexError(self, pos.x, pos.y);
    }

    if(py_isinstance(&argv[1], tp_array2d_like)) {
        c11_array2d_like* mask = py_touserdata(&argv[1]);
        if(!_array2d_like_check_same_shape(self, mask)) return false;
        for(int j = 0; j < self->n_rows; j++) {
            for(int i = 0; i < self->n_cols; i++) {
                py_Ref cond = mask->f_get(mask, i, j);
                if(!py_checkbool(cond)) return false;
                if(py_tobool(cond)) {
                    bool ok = self->f_set(self, i, j, value);
                    if(!ok) return false;
                }
            }
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
        if(c11_array2d_like_is_valid(self, col, row)) {
            bool ok = self->f_set(self, col, row, value);
            if(!ok) return false;
            py_newnone(py_retval());
            return true;
        }
        return _array2d_like_IndexError(self, col, row);
    }

    bool _1 = py_istype(x, tp_slice) && py_istype(y, tp_slice);
    bool _2 = py_istype(x, tp_int) && py_istype(y, tp_slice);
    bool _3 = py_istype(x, tp_slice) && py_istype(y, tp_int);
    if(_1 || _2 || _3) {
        HANDLE_SLICE();
        if(py_isinstance(value, tp_array2d_like)) {
            c11_array2d_like* values = py_touserdata(value);
            if(!_check_same_shape(slice_width, slice_height, values->n_cols, values->n_rows))
                return false;
            for(int j = 0; j < slice_height; j++) {
                for(int i = 0; i < slice_width; i++) {
                    py_Ref item = values->f_get(values, i, j);
                    bool ok = self->f_set(self, start_col + i, start_row + j, item);
                    if(!ok) return false;
                }
            }
        } else {
            for(int j = 0; j < slice_height; j++) {
                for(int i = 0; i < slice_width; i++) {
                    bool ok = self->f_set(self, start_col + i, start_row + j, value);
                    if(!ok) return false;
                }
            }
        }
        py_newnone(py_retval());
        return true;
    }

    return TypeError("expected tuple[int, int] or tuple[slice, slice]");
}

// count(self, value: T) -> int
static bool array2d_like_count(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_array2d_like* self = py_touserdata(argv);
    int count = 0;
    for(int j = 0; j < self->n_rows; j++) {
        for(int i = 0; i < self->n_cols; i++) {
            int code = py_equal(self->f_get(self, i, j), py_arg(1));
            if(code == -1) return false;
            count += code;
        }
    }
    py_newint(py_retval(), count);
    return true;
}

// get_bounding_rect(self, value: T) -> tuple[int, int, int, int]
static bool array2d_like_get_bounding_rect(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_array2d_like* self = py_touserdata(argv);
    py_Ref value = py_arg(1);
    int left = self->n_cols;
    int top = self->n_rows;
    int right = 0;
    int bottom = 0;
    for(int j = 0; j < self->n_rows; j++) {
        for(int i = 0; i < self->n_cols; i++) {
            py_Ref item = self->f_get(self, i, j);
            int res = py_equal(item, value);
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
        py_TValue* data = py_newtuple(py_retval(), 4);
        py_newint(&data[0], left);
        py_newint(&data[1], top);
        py_newint(&data[2], width);
        py_newint(&data[3], height);
    }
    return true;
}

// count_neighbors(self, value: T, neighborhood: Neighborhood) -> array2d[int]
static bool array2d_like_count_neighbors(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    c11_array2d_like* self = py_touserdata(argv);
    c11_array2d* res = c11_newarray2d(py_pushtmp(), self->n_cols, self->n_rows);
    py_Ref value = py_arg(1);
    const char* neighborhood = py_tostr(py_arg(2));

    const static c11_vec2i Moore[] = {
        {{-1, -1}},
        {{0, -1}},
        {{1, -1}},
        {{-1, 0}},
        {{1, 0}},
        {{-1, 1}},
        {{0, 1}},
        {{1, 1}},
    };

    const static c11_vec2i von_Neumann[] = {
        {{0, -1}},
        {{-1, 0}},
        {{1, 0}},
        {{0, 1}},
    };

    const c11_vec2i* offsets;
    int n_offsets;
    if(strcmp(neighborhood, "Moore") == 0) {
        offsets = Moore;
        n_offsets = c11__count_array(Moore);
    } else if(strcmp(neighborhood, "von Neumann") == 0) {
        offsets = von_Neumann;
        n_offsets = c11__count_array(von_Neumann);
    } else {
        return ValueError("neighborhood must be 'Moore' or 'von Neumann'");
    }
    for(int j = 0; j < self->n_rows; j++) {
        for(int i = 0; i < self->n_cols; i++) {
            py_i64 count = 0;
            for(int k = 0; k < n_offsets; k++) {
                int x = i + offsets[k].x;
                int y = j + offsets[k].y;
                if(x >= 0 && x < self->n_cols && y >= 0 && y < self->n_rows) {
                    py_Ref item = self->f_get(self, x, y);
                    int code = py_equal(item, value);
                    if(code == -1) return false;
                    count += code;
                }
            }
            py_newint(c11_array2d__get(res, i, j), count);
        }
    }
    py_assign(py_retval(), py_peek(-1));
    py_pop();
    return true;
}

// convolve(self: array2d_like[int], kernel: array2d_like[int], padding: int) -> array2d[int]
static bool array2d_like_convolve(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    if(!py_checkinstance(&argv[1], tp_array2d_like)) return false;
    PY_CHECK_ARG_TYPE(2, tp_int);
    c11_array2d_like* self = py_touserdata(&argv[0]);
    c11_array2d_like* kernel = py_touserdata(&argv[1]);
    int padding = py_toint(py_arg(2));
    if(kernel->n_cols != kernel->n_rows) return ValueError("kernel must be square");
    int ksize = kernel->n_cols;
    if(ksize % 2 == 0) return ValueError("kernel size must be odd");
    int ksize_half = ksize / 2;
    c11_array2d* res = c11_newarray2d(py_pushtmp(), self->n_cols, self->n_rows);
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
                        py_Ref item = self->f_get(self, x, y);
                        if(!py_checkint(item)) return false;
                        _0 = py_toint(item);
                    }
                    py_Ref kitem = kernel->f_get(kernel, ii, jj);
                    if(!py_checkint(kitem)) return false;
                    _1 = py_toint(kitem);
                    sum += _0 * _1;
                }
            }
            py_newint(c11_array2d__get(res, i, j), sum);
        }
    }
    py_assign(py_retval(), py_peek(-1));
    py_pop();
    return true;
}

#undef HANDLE_SLICE

static void register_array2d_like(py_Ref mod) {
    py_Type type = py_newtype("array2d_like", tp_object, mod, NULL);
    assert(type == tp_array2d_like);

    py_bindproperty(type, "n_cols", array2d_like_n_cols, NULL);
    py_bindproperty(type, "n_rows", array2d_like_n_rows, NULL);
    py_bindproperty(type, "width", array2d_like_n_cols, NULL);
    py_bindproperty(type, "height", array2d_like_n_rows, NULL);
    py_bindproperty(type, "shape", array2d_like_shape, NULL);
    py_bindproperty(type, "numel", array2d_like_numel, NULL);

    py_bindmethod(type, "is_valid", array2d_like_is_valid);
    py_bindmethod(type, "get", array2d_like_get);
    py_bindmethod(type, "index", array2d_like_index);

    py_bindmethod(type, "render", array2d_like_render);

    py_bindmethod(type, "all", array2d_like_all);
    py_bindmethod(type, "any", array2d_like_any);

    py_bindmethod(type, "map", array2d_like_map);
    py_bindmethod(type, "apply", array2d_like_apply);
    py_bindmethod(type, "zip_with", array2d_like_zip_with);
    py_bindmethod(type, "copy", array2d_like_copy);
    py_bindmethod(type, "tolist", array2d_like_tolist);

    py_bindmagic(type, __le__, array2d_like__le__);
    py_bindmagic(type, __lt__, array2d_like__lt__);
    py_bindmagic(type, __ge__, array2d_like__ge__);
    py_bindmagic(type, __gt__, array2d_like__gt__);
    py_bindmagic(type, __eq__, array2d_like__eq__);
    py_bindmagic(type, __ne__, array2d_like__ne__);

    py_bindmagic(type, __add__, array2d_like__add__);
    py_bindmagic(type, __sub__, array2d_like__sub__);
    py_bindmagic(type, __mul__, array2d_like__mul__);
    py_bindmagic(type, __truediv__, array2d_like__truediv__);
    py_bindmagic(type, __floordiv__, array2d_like__floordiv__);
    py_bindmagic(type, __mod__, array2d_like__mod__);
    py_bindmagic(type, __pow__, array2d_like__pow__);

    py_bindmagic(type, __and__, array2d_like__and__);
    py_bindmagic(type, __or__, array2d_like__or__);
    py_bindmagic(type, __xor__, array2d_like__xor__);
    py_bindmagic(type, __invert__, array2d_like__invert__);

    py_bindmagic(type, __iter__, array2d_like__iter__);
    py_bindmagic(type, __repr__, array2d_like__repr__);

    py_bindmagic(type, __getitem__, array2d_like__getitem__);
    py_bindmagic(type, __setitem__, array2d_like__setitem__);

    py_bindmethod(type, "count", array2d_like_count);
    py_bindmethod(type, "get_bounding_rect", array2d_like_get_bounding_rect);
    py_bindmethod(type, "count_neighbors", array2d_like_count_neighbors);
    py_bindmethod(type, "convolve", array2d_like_convolve);

    const char* scc =
        "\ndef get_connected_components(self, value: T, neighborhood: Neighborhood) -> tuple[array2d[int], int]:\n    from collections import deque\n    from vmath import vec2i\n\n    DIRS = [vec2i.LEFT, vec2i.RIGHT, vec2i.UP, vec2i.DOWN]\n    assert neighborhood in ['Moore', 'von Neumann']\n\n    if neighborhood == 'Moore':\n        DIRS.extend([\n            vec2i.LEFT+vec2i.UP,\n            vec2i.RIGHT+vec2i.UP,\n            vec2i.LEFT+vec2i.DOWN,\n            vec2i.RIGHT+vec2i.DOWN\n            ])\n\n    visited = array2d[int](self.width, self.height, default=0)\n    queue = deque()\n    count = 0\n    for y in range(self.height):\n        for x in range(self.width):\n            if visited[x, y] or self[x, y] != value:\n                continue\n            count += 1\n            queue.append((x, y))\n            visited[x, y] = count\n            while queue:\n                cx, cy = queue.popleft()\n                for dx, dy in DIRS:\n                    nx, ny = cx+dx, cy+dy\n                    if self.is_valid(nx, ny) and not visited[nx, ny] and self[nx, ny] == value:\n                        queue.append((nx, ny))\n                        visited[nx, ny] = count\n    return visited, count\n\narray2d_like.get_connected_components = get_connected_components\ndel get_connected_components\n";
    if(!py_exec(scc, "array2d.py", EXEC_MODE, mod)) {
        py_printexc();
        c11__abort("failed to execute array2d.py");
    }
}

bool array2d_like_iterator__next__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_array2d_like_iterator* self = py_touserdata(argv);
    if(self->j >= self->array->n_rows) return StopIteration();
    py_TValue* data = py_newtuple(py_retval(), 2);
    py_newvec2i(&data[0],
                (c11_vec2i){
                    {self->i, self->j}
    });
    py_assign(&data[1], self->array->f_get(self->array, self->i, self->j));
    self->i++;
    if(self->i >= self->array->n_cols) {
        self->i = 0;
        self->j++;
    }
    return true;
}

static void register_array2d_like_iterator(py_Ref mod) {
    py_Type type = py_newtype("array2d_like_iterator", tp_object, mod, NULL);
    assert(type == tp_array2d_like_iterator);
    py_bindmagic(type, __iter__, pk_wrapper__self);
    py_bindmagic(type, __next__, array2d_like_iterator__next__);
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
    c11_array2d* ud = c11_newarray2d(py_pushtmp(), n_cols, n_rows);
    // setup initial values
    if(py_callable(default_)) {
        for(int j = 0; j < n_rows; j++) {
            for(int i = 0; i < n_cols; i++) {
                py_TValue tmp;
                py_newvec2i(&tmp,
                            (c11_vec2i){
                                {i, j}
                });
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
    c11_array2d* res = c11_newarray2d(py_retval(), n_cols, n_rows);
    for(int j = 0; j < n_rows; j++) {
        py_Ref row_j = py_list_getitem(argv, j);
        for(int i = 0; i < n_cols; i++) {
            c11_array2d__set(res, i, j, py_list_getitem(row_j, i));
        }
    }
    return true;
}

static void register_array2d(py_Ref mod) {
    py_Type type = py_newtype("array2d", tp_array2d_like, mod, NULL);
    assert(type == tp_array2d);
    py_bind(py_tpobject(type),
            "__new__(cls, n_cols: int, n_rows: int, default=None)",
            array2d__new__);
    py_bindstaticmethod(type, "fromlist", array2d_fromlist_STATIC);
}

static bool array2d_view_origin(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_array2d_view* self = py_touserdata(argv);
    py_newvec2i(py_retval(), self->origin);
    return true;
}

static void register_array2d_view(py_Ref mod) {
    py_Type type = py_newtype("array2d_view", tp_array2d_like, mod, NULL);
    assert(type == tp_array2d_view);
    py_bindproperty(type, "origin", array2d_view_origin, NULL);
}

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
        if(!ok) {
            PK_FREE(data);
            return NULL;
        }
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

static void
    cpy11__divmod_int_uint(int a, int b_log2, int b_mask, int* restrict q, int* restrict r) {
    if(a >= 0) {
        *q = a >> b_log2;
        *r = a & b_mask;
    } else {
        *q = -1 - ((-a - 1) >> b_log2);
        *r = b_mask - ((-a - 1) & b_mask);
    }
}

static void c11_chunked_array2d__world_to_chunk(c11_chunked_array2d* self,
                                                int col,
                                                int row,
                                                c11_vec2i* restrict chunk_pos,
                                                c11_vec2i* restrict local_pos) {
    cpy11__divmod_int_uint(col,
                           self->chunk_size_log2,
                           self->chunk_size_mask,
                           &chunk_pos->x,
                           &local_pos->x);
    cpy11__divmod_int_uint(row,
                           self->chunk_size_log2,
                           self->chunk_size_mask,
                           &chunk_pos->y,
                           &local_pos->y);
}

static py_TValue* c11_chunked_array2d__parse_col_row(c11_chunked_array2d* self,
                                                     int col,
                                                     int row,
                                                     c11_vec2i* restrict chunk_pos,
                                                     c11_vec2i* restrict local_pos) {
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
    return data;
}

py_Ref c11_chunked_array2d__get(c11_chunked_array2d* self, int col, int row) {
    c11_vec2i chunk_pos, local_pos;
    py_TValue* data = c11_chunked_array2d__parse_col_row(self, col, row, &chunk_pos, &local_pos);
    if(data == NULL) return &self->default_T;
    py_Ref retval = &data[1 + local_pos.y * self->chunk_size + local_pos.x];
    if(py_isnil(retval)) return &self->default_T;
    return retval;
}

bool c11_chunked_array2d__set(c11_chunked_array2d* self, int col, int row, py_Ref value) {
    c11_vec2i chunk_pos, local_pos;
    py_TValue* data = c11_chunked_array2d__parse_col_row(self, col, row, &chunk_pos, &local_pos);
    if(data == NULL) {
        data = c11_chunked_array2d__new_chunk(self, chunk_pos);
        if(data == NULL) return false;
    }
    data[1 + local_pos.y * self->chunk_size + local_pos.x] = *value;
    return true;
}

static void c11_chunked_array2d__del(c11_chunked_array2d* self, int col, int row) {
    c11_vec2i chunk_pos, local_pos;
    py_TValue* data = c11_chunked_array2d__parse_col_row(self, col, row, &chunk_pos, &local_pos);
    if(data != NULL) data[1 + local_pos.y * self->chunk_size + local_pos.x] = *py_NIL();
}

static bool chunked_array2d__new__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(4);
    PY_CHECK_ARG_TYPE(1, tp_int);
    py_Type cls = py_totype(argv);
    c11_chunked_array2d* self = py_newobject(py_retval(), cls, 0, sizeof(c11_chunked_array2d));
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
        default: return ValueError("invalid chunk_size: %d, not power of 2", chunk_size);
    }
    self->chunk_size_mask = chunk_size - 1;
    memset(&self->last_visited, 0, sizeof(c11_chunked_array2d_chunks_KV));
    return true;
}

static bool chunked_array2d_chunk_size(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_chunked_array2d* self = py_touserdata(argv);
    py_newint(py_retval(), self->chunk_size);
    return true;
}

static bool chunked_array2d_default(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_chunked_array2d* self = py_touserdata(argv);
    py_assign(py_retval(), &self->default_T);
    return true;
}

static bool chunked_array2d_context_builder(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_chunked_array2d* self = py_touserdata(argv);
    py_assign(py_retval(), &self->context_builder);
    return true;
}

static bool chunked_array2d__getitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_vec2i);
    c11_chunked_array2d* self = py_touserdata(argv);
    c11_vec2i pos = py_tovec2i(&argv[1]);
    py_Ref res = c11_chunked_array2d__get(self, pos.x, pos.y);
    py_assign(py_retval(), res);
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
    py_Ref data = py_newtuple(py_pushtmp(), self->chunks.length);
    for(int i = 0; i < self->chunks.length; i++) {
        c11_chunked_array2d_chunks_KV* kv =
            c11__at(c11_chunked_array2d_chunks_KV, &self->chunks, i);
        py_Ref p = py_newtuple(&data[i], 2);
        py_newvec2i(&p[0], kv->key);  // pos
        p[1] = kv->value[0];          // context
    }
    bool ok = py_iter(py_peek(-1));
    if(!ok) return false;
    py_pop();
    return true;
}

static bool chunked_array2d__len__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_chunked_array2d* self = py_touserdata(argv);
    py_newint(py_retval(), self->chunks.length);
    return true;
}

static bool chunked_array2d_clear(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_chunked_array2d* self = py_touserdata(argv);
    c11__foreach(c11_chunked_array2d_chunks_KV, &self->chunks, p_kv) PK_FREE(p_kv->value);
    c11_chunked_array2d_chunks__clear(&self->chunks);
    self->last_visited.value = NULL;
    py_newnone(py_retval());
    return true;
}

static bool chunked_array2d_copy(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_chunked_array2d* self = py_touserdata(argv);
    c11_chunked_array2d* res =
        py_newobject(py_retval(), tp_chunked_array2d, 0, sizeof(c11_chunked_array2d));
    // copy basic data
    memcpy(res, self, sizeof(c11_chunked_array2d));
    // invalidate last_visited cache
    self->last_visited.value = NULL;
    // copy chunks
    memset(&res->chunks, 0, sizeof(c11_chunked_array2d_chunks));
    c11_chunked_array2d_chunks__ctor(&res->chunks);
    c11_vector__reserve(&res->chunks, self->chunks.capacity);
    for(int i = 0; i < self->chunks.length; i++) {
        c11_chunked_array2d_chunks_KV* kv =
            c11__at(c11_chunked_array2d_chunks_KV, &self->chunks, i);
        int chunk_numel = self->chunk_size * self->chunk_size + 1;
        py_TValue* data = PK_MALLOC(sizeof(py_TValue) * chunk_numel);
        memcpy(data, kv->value, sizeof(py_TValue) * chunk_numel);
        // construct new KV
        c11_chunked_array2d_chunks_KV new_kv;
        new_kv.key = kv->key;
        new_kv.value = data;
        c11_vector__push(c11_chunked_array2d_chunks_KV, &res->chunks, new_kv);
    }
    return true;
}

static bool chunked_array2d_world_to_chunk(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_vec2i);
    c11_chunked_array2d* self = py_touserdata(argv);
    c11_vec2i pos = py_tovec2i(&argv[1]);
    c11_vec2i chunk_pos, local_pos;
    c11_chunked_array2d__world_to_chunk(self, pos.x, pos.y, &chunk_pos, &local_pos);
    py_TValue* p = py_newtuple(py_retval(), 2);
    py_newvec2i(&p[0], chunk_pos);
    py_newvec2i(&p[1], local_pos);
    return true;
}

static bool chunked_array2d_add_chunk(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_vec2i);
    c11_chunked_array2d* self = py_touserdata(argv);
    c11_vec2i pos = py_tovec2i(&argv[1]);
    py_TValue* data = c11_chunked_array2d__new_chunk(self, pos);
    if(data == NULL) return false;
    py_assign(py_retval(), &data[0]);  // context
    return true;
}

static bool chunked_array2d_remove_chunk(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_vec2i);
    c11_chunked_array2d* self = py_touserdata(argv);
    c11_vec2i pos = py_tovec2i(&argv[1]);
    py_TValue* data = c11_chunked_array2d_chunks__get(&self->chunks, pos, NULL);
    if(data != NULL) {
        PK_FREE(data);
        bool ok = c11_chunked_array2d_chunks__del(&self->chunks, pos);
        assert(ok);
        self->last_visited.value = NULL;
        py_newbool(py_retval(), ok);
    } else {
        py_newbool(py_retval(), false);
    }
    return true;
}

static bool chunked_array2d_move_chunk(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(1, tp_vec2i);
    PY_CHECK_ARG_TYPE(2, tp_vec2i);
    c11_chunked_array2d* self = py_touserdata(argv);
    c11_vec2i src = py_tovec2i(&argv[1]);
    c11_vec2i dst = py_tovec2i(&argv[2]);
    py_TValue* src_data = c11_chunked_array2d_chunks__get(&self->chunks, src, NULL);
    py_TValue* dst_data = c11_chunked_array2d_chunks__get(&self->chunks, dst, NULL);
    if(src_data == NULL || dst_data != NULL) {
        py_newbool(py_retval(), false);
        return true;
    }
    c11_chunked_array2d_chunks__del(&self->chunks, src);
    c11_chunked_array2d_chunks__set(&self->chunks, dst, src_data);
    self->last_visited.value = NULL;
    py_newbool(py_retval(), true);
    return true;
}

static bool chunked_array2d_get_context(int argc, py_Ref argv) {
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

void c11_chunked_array2d__mark(void* ud, c11_vector* p_stack) {
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

static bool chunked_array2d_view(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_chunked_array2d* self = py_touserdata(&argv[0]);
    if(self->chunks.length == 0) { return ValueError("chunked_array2d is empty"); }
    int min_chunk_x = INT_MAX;
    int min_chunk_y = INT_MAX;
    int max_chunk_x = INT_MIN;
    int max_chunk_y = INT_MIN;
    for(int i = 0; i < self->chunks.length; i++) {
        c11_vec2i chunk_pos = c11__getitem(c11_chunked_array2d_chunks_KV, &self->chunks, i).key;
        min_chunk_x = c11__min(min_chunk_x, chunk_pos.x);
        min_chunk_y = c11__min(min_chunk_y, chunk_pos.y);
        max_chunk_x = c11__max(max_chunk_x, chunk_pos.x);
        max_chunk_y = c11__max(max_chunk_y, chunk_pos.y);
    }
    int start_col = min_chunk_x * self->chunk_size;
    int start_row = min_chunk_y * self->chunk_size;
    int width = (max_chunk_x - min_chunk_x + 1) * self->chunk_size;
    int height = (max_chunk_y - min_chunk_y + 1) * self->chunk_size;
    return _chunked_array2d_view(py_retval(), argv, self, start_col, start_row, width, height);
}

static bool chunked_array2d_view_rect(int argc, py_Ref argv) {
    PY_CHECK_ARGC(4);
    PY_CHECK_ARG_TYPE(1, tp_vec2i);
    PY_CHECK_ARG_TYPE(2, tp_int);
    PY_CHECK_ARG_TYPE(3, tp_int);
    c11_chunked_array2d* self = py_touserdata(&argv[0]);
    c11_vec2i pos = py_tovec2i(&argv[1]);
    int width = py_toint(&argv[2]);
    int height = py_toint(&argv[3]);
    return _chunked_array2d_view(py_retval(), argv, self, pos.x, pos.y, width, height);
}

static bool chunked_array2d_view_chunk(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_vec2i);
    c11_chunked_array2d* self = py_touserdata(&argv[0]);
    c11_vec2i chunk_pos = py_tovec2i(&argv[1]);
    int start_col = chunk_pos.x * self->chunk_size;
    int start_row = chunk_pos.y * self->chunk_size;
    return _chunked_array2d_view(py_retval(),
                                 argv,
                                 self,
                                 start_col,
                                 start_row,
                                 self->chunk_size,
                                 self->chunk_size);
}

static bool chunked_array2d_view_chunks(int argc, py_Ref argv) {
    PY_CHECK_ARGC(4);
    PY_CHECK_ARG_TYPE(1, tp_vec2i);
    PY_CHECK_ARG_TYPE(2, tp_int);
    PY_CHECK_ARG_TYPE(3, tp_int);
    c11_chunked_array2d* self = py_touserdata(&argv[0]);
    c11_vec2i chunk_pos = py_tovec2i(&argv[1]);
    int width = py_toint(&argv[2]) * self->chunk_size;
    int height = py_toint(&argv[3]) * self->chunk_size;
    int start_col = chunk_pos.x * self->chunk_size;
    int start_row = chunk_pos.y * self->chunk_size;
    return _chunked_array2d_view(py_retval(), argv, self, start_col, start_row, width, height);
}

static void register_chunked_array2d(py_Ref mod) {
    py_Type type =
        py_newtype("chunked_array2d", tp_object, mod, (py_Dtor)c11_chunked_array2d__dtor);
    assert(type == tp_chunked_array2d);

    py_bind(py_tpobject(type),
            "__new__(cls, chunk_size, default=None, context_builder=None)",
            chunked_array2d__new__);

    py_bindproperty(type, "chunk_size", chunked_array2d_chunk_size, NULL);
    py_bindproperty(type, "default", chunked_array2d_default, NULL);
    py_bindproperty(type, "context_builder", chunked_array2d_context_builder, NULL);

    py_bindmagic(type, __getitem__, chunked_array2d__getitem__);
    py_bindmagic(type, __setitem__, chunked_array2d__setitem__);
    py_bindmagic(type, __delitem__, chunked_array2d__delitem__);
    py_bindmagic(type, __iter__, chunked_array2d__iter__);
    py_bindmagic(type, __len__, chunked_array2d__len__);

    py_bindmethod(type, "clear", chunked_array2d_clear);
    py_bindmethod(type, "copy", chunked_array2d_copy);
    py_bindmethod(type, "world_to_chunk", chunked_array2d_world_to_chunk);
    py_bindmethod(type, "add_chunk", chunked_array2d_add_chunk);
    py_bindmethod(type, "remove_chunk", chunked_array2d_remove_chunk);
    py_bindmethod(type, "move_chunk", chunked_array2d_move_chunk);
    py_bindmethod(type, "get_context", chunked_array2d_get_context);

    py_bindmethod(type, "view", chunked_array2d_view);
    py_bindmethod(type, "view_rect", chunked_array2d_view_rect);
    py_bindmethod(type, "view_chunk", chunked_array2d_view_chunk);
    py_bindmethod(type, "view_chunks", chunked_array2d_view_chunks);
}

void pk__add_module_array2d() {
    py_GlobalRef mod = py_newmodule("array2d");

    register_array2d_like(mod);
    register_array2d_like_iterator(mod);
    register_array2d(mod);
    register_array2d_view(mod);
    register_chunked_array2d(mod);
}

void py_newarray2d(py_OutRef out, int width, int height) { c11_newarray2d(out, width, height); }

int py_array2d_getwidth(py_Ref self) {
    assert(self->type == tp_array2d);
    c11_array2d* ud = py_touserdata(self);
    return ud->header.n_cols;
}

int py_array2d_getheight(py_Ref self) {
    assert(self->type == tp_array2d);
    c11_array2d* ud = py_touserdata(self);
    return ud->header.n_rows;
}

py_ObjectRef py_array2d_getitem(py_Ref self, int x, int y) {
    assert(self->type == tp_array2d);
    c11_array2d* ud = py_touserdata(self);
    return c11_array2d__get(ud, x, y);
}

void py_array2d_setitem(py_Ref self, int x, int y, py_Ref value) {
    assert(self->type == tp_array2d);
    c11_array2d* ud = py_touserdata(self);
    c11_array2d__set(ud, x, y, value);
}