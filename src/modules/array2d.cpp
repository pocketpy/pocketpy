#include "pocketpy/modules/array2d.hpp"
#include "pocketpy/interpreter/bindings.hpp"

namespace pkpy {

struct Array2d {
    PK_ALWAYS_PASS_BY_POINTER(Array2d)

    PyVar* data;
    int n_cols;
    int n_rows;
    int numel;

    Array2d() {
        data = nullptr;
        n_cols = 0;
        n_rows = 0;
        numel = 0;
    }

    void init(int n_cols, int n_rows) {
        this->n_cols = n_cols;
        this->n_rows = n_rows;
        this->numel = n_cols * n_rows;
        this->data = new PyVar[numel];
    }

    bool is_valid(int col, int row) const { return 0 <= col && col < n_cols && 0 <= row && row < n_rows; }

    void check_valid(VM* vm, int col, int row) const {
        if(is_valid(col, row)) {
            return;
        }
        vm->IndexError(_S('(', col, ", ", row, ')', " is not a valid index for array2d(", n_cols, ", ", n_rows, ')'));
    }

    PyVar _get(int col, int row) { return data[row * n_cols + col]; }

    void _set(int col, int row, PyVar value) { data[row * n_cols + col] = value; }

    static void _register(VM* vm, PyObject* mod, PyObject* type) {
        vm->bind(type, "__new__(cls, *args, **kwargs)", [](VM* vm, ArgsView args) {
            Type cls = PK_OBJ_GET(Type, args[0]);
            return vm->new_object<Array2d>(cls);
        });

        vm->bind(type, "__init__(self, n_cols: int, n_rows: int, default=None)", [](VM* vm, ArgsView args) {
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            int n_cols = CAST(int, args[1]);
            int n_rows = CAST(int, args[2]);
            if(n_cols <= 0 || n_rows <= 0) {
                vm->ValueError("n_cols and n_rows must be positive integers");
            }
            self.init(n_cols, n_rows);
            if(vm->py_callable(args[3])) {
                for(int i = 0; i < self.numel; i++) {
                    self.data[i] = vm->call(args[3]);
                }
            } else {
                for(int i = 0; i < self.numel; i++) {
                    self.data[i] = args[3];
                }
            }
            return vm->None;
        });

        PY_READONLY_FIELD(Array2d, "n_cols", n_cols);
        PY_READONLY_FIELD(Array2d, "n_rows", n_rows);
        PY_READONLY_FIELD(Array2d, "width", n_cols);
        PY_READONLY_FIELD(Array2d, "height", n_rows);
        PY_READONLY_FIELD(Array2d, "numel", numel);

        // _get
        vm->bind_func(type, "_get", 3, [](VM* vm, ArgsView args) {
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            int col = CAST(int, args[1]);
            int row = CAST(int, args[2]);
            self.check_valid(vm, col, row);
            return self._get(col, row);
        });

        // _set
        vm->bind_func(type, "_set", 4, [](VM* vm, ArgsView args) {
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            int col = CAST(int, args[1]);
            int row = CAST(int, args[2]);
            self.check_valid(vm, col, row);
            self._set(col, row, args[3]);
            return vm->None;
        });

        vm->bind_func(type, "is_valid", 3, [](VM* vm, ArgsView args) {
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            int col = CAST(int, args[1]);
            int row = CAST(int, args[2]);
            return VAR(self.is_valid(col, row));
        });

        vm->bind(type, "get(self, col: int, row: int, default=None)", [](VM* vm, ArgsView args) {
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            int col = CAST(int, args[1]);
            int row = CAST(int, args[2]);
            if(!self.is_valid(col, row)) {
                return args[3];
            }
            return self._get(col, row);
        });

#define HANDLE_SLICE()                                                                                                 \
    int start_col, stop_col, step_col;                                                                                 \
    int start_row, stop_row, step_row;                                                                                 \
    vm->parse_int_slice(PK_OBJ_GET(Slice, xy[0]), self.n_cols, start_col, stop_col, step_col);                         \
    vm->parse_int_slice(PK_OBJ_GET(Slice, xy[1]), self.n_rows, start_row, stop_row, step_row);                         \
    if(step_col != 1 || step_row != 1)                                                                                 \
        vm->ValueError("slice step must be 1");                                                                        \
    int slice_width = stop_col - start_col;                                                                            \
    int slice_height = stop_row - start_row;                                                                           \
    if(slice_width <= 0 || slice_height <= 0)                                                                          \
        vm->ValueError("slice width and height must be positive");

        vm->bind__getitem__(type->as<Type>(), [](VM* vm, PyVar _0, PyVar _1) {
            Array2d& self = PK_OBJ_GET(Array2d, _0);
            const Tuple& xy = CAST(Tuple&, _1);

            if(is_int(xy[0]) && is_int(xy[1])) {
                i64 col = xy[0].as<i64>();
                i64 row = xy[1].as<i64>();
                self.check_valid(vm, col, row);
                return self._get(col, row);
            }

            if(is_type(xy[0], VM::tp_slice) && is_type(xy[1], VM::tp_slice)) {
                HANDLE_SLICE();
                PyVar new_array_obj = vm->new_user_object<Array2d>();
                Array2d& new_array = PK_OBJ_GET(Array2d, new_array_obj);
                new_array.init(stop_col - start_col, stop_row - start_row);
                for(int j = start_row; j < stop_row; j++) {
                    for(int i = start_col; i < stop_col; i++) {
                        new_array._set(i - start_col, j - start_row, self._get(i, j));
                    }
                }
                return new_array_obj;
            }
            vm->TypeError("expected `tuple[int, int]` or `tuple[slice, slice]` as index");
        });

        vm->bind__setitem__(type->as<Type>(), [](VM* vm, PyVar _0, PyVar _1, PyVar _2) {
            Array2d& self = PK_OBJ_GET(Array2d, _0);
            const Tuple& xy = CAST(Tuple&, _1);
            if(is_int(xy[0]) && is_int(xy[1])) {
                i64 col = xy[0].as<i64>();
                i64 row = xy[1].as<i64>();
                self.check_valid(vm, col, row);
                self._set(col, row, _2);
                return;
            }

            if(is_type(xy[0], VM::tp_slice) && is_type(xy[1], VM::tp_slice)) {
                HANDLE_SLICE();

                bool is_basic_type = false;
                switch(vm->_tp(_2).index) {
                    case VM::tp_int.index: is_basic_type = true; break;
                    case VM::tp_float.index: is_basic_type = true; break;
                    case VM::tp_str.index: is_basic_type = true; break;
                    case VM::tp_bool.index: is_basic_type = true; break;
                    default: is_basic_type = _2 == vm->None;
                }

                if(is_basic_type) {
                    for(int j = 0; j < slice_height; j++) {
                        for(int i = 0; i < slice_width; i++) {
                            self._set(i + start_col, j + start_row, _2);
                        }
                    }
                    return;
                }

                if(!vm->is_user_type<Array2d>(_2)) {
                    vm->TypeError(_S("expected int/float/str/bool/None or an array2d instance"));
                }

                Array2d& other = PK_OBJ_GET(Array2d, _2);
                if(slice_width != other.n_cols || slice_height != other.n_rows) {
                    vm->ValueError("array2d size does not match the slice size");
                }
                for(int j = 0; j < slice_height; j++) {
                    for(int i = 0; i < slice_width; i++) {
                        self._set(i + start_col, j + start_row, other._get(i, j));
                    }
                }
                return;
            }
            vm->TypeError("expected `tuple[int, int]` or `tuple[slice, slice]` as index");
        });

#undef HANDLE_SLICE

        vm->bind_func(type, "tolist", 1, [](VM* vm, ArgsView args) {
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            List t(self.n_rows);
            for(int j = 0; j < self.n_rows; j++) {
                List row(self.n_cols);
                for(int i = 0; i < self.n_cols; i++) {
                    row[i] = self._get(i, j);
                }
                t[j] = VAR(std::move(row));
            }
            return VAR(std::move(t));
        });

        vm->bind__len__(type->as<Type>(), [](VM* vm, PyVar _0) {
            Array2d& self = PK_OBJ_GET(Array2d, _0);
            return (i64)self.numel;
        });

        vm->bind__repr__(type->as<Type>(), [](VM* vm, PyVar _0) -> Str {
            Array2d& self = PK_OBJ_GET(Array2d, _0);
            return _S("array2d(", self.n_cols, ", ", self.n_rows, ')');
        });

        vm->bind_func(type, "map", 2, [](VM* vm, ArgsView args) {
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            PyVar f = args[1];
            PyVar new_array_obj = vm->new_user_object<Array2d>();
            Array2d& new_array = PK_OBJ_GET(Array2d, new_array_obj);
            new_array.init(self.n_cols, self.n_rows);
            for(int i = 0; i < new_array.numel; i++) {
                new_array.data[i] = vm->call(f, self.data[i]);
            }
            return new_array_obj;
        });

        vm->bind_func(type, "copy", 1, [](VM* vm, ArgsView args) {
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            PyVar new_array_obj = vm->new_user_object<Array2d>();
            Array2d& new_array = PK_OBJ_GET(Array2d, new_array_obj);
            new_array.init(self.n_cols, self.n_rows);
            for(int i = 0; i < new_array.numel; i++) {
                new_array.data[i] = self.data[i];
            }
            return new_array_obj;
        });

        vm->bind_func(type, "fill_", 2, [](VM* vm, ArgsView args) {
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            for(int i = 0; i < self.numel; i++) {
                self.data[i] = args[1];
            }
            return vm->None;
        });

        vm->bind_func(type, "apply_", 2, [](VM* vm, ArgsView args) {
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            PyVar f = args[1];
            for(int i = 0; i < self.numel; i++) {
                self.data[i] = vm->call(f, self.data[i]);
            }
            return vm->None;
        });

        vm->bind_func(type, "copy_", 2, [](VM* vm, ArgsView args) {
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            if(is_type(args[1], VM::tp_list)) {
                const List& list = PK_OBJ_GET(List, args[1]);
                if(list.size() != self.numel) {
                    vm->ValueError("list size must be equal to the number of elements in the array2d");
                }
                for(int i = 0; i < self.numel; i++) {
                    self.data[i] = list[i];
                }
                return vm->None;
            }
            Array2d& other = CAST(Array2d&, args[1]);
            // if self and other have different sizes, re-initialize self
            if(self.n_cols != other.n_cols || self.n_rows != other.n_rows) {
                delete self.data;
                self.init(other.n_cols, other.n_rows);
            }
            for(int i = 0; i < self.numel; i++) {
                self.data[i] = other.data[i];
            }
            return vm->None;
        });

        vm->bind__eq__(type->as<Type>(), [](VM* vm, PyVar _0, PyVar _1) {
            Array2d& self = PK_OBJ_GET(Array2d, _0);
            if(!vm->is_user_type<Array2d>(_1)) {
                return vm->NotImplemented;
            }
            Array2d& other = PK_OBJ_GET(Array2d, _1);
            if(self.n_cols != other.n_cols || self.n_rows != other.n_rows) {
                return vm->False;
            }
            for(int i = 0; i < self.numel; i++) {
                if(vm->py_ne(self.data[i], other.data[i])) {
                    return vm->False;
                }
            }
            return vm->True;
        });

        vm->bind(type, "count_neighbors(self, value, neighborhood='Moore') -> array2d[int]", [](VM* vm, ArgsView args) {
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            PyVar new_array_obj = vm->new_user_object<Array2d>();
            Array2d& new_array = PK_OBJ_GET(Array2d, new_array_obj);
            new_array.init(self.n_cols, self.n_rows);
            PyVar value = args[1];
            const Str& neighborhood = CAST(Str&, args[2]);
            if(neighborhood == "Moore") {
                for(int j = 0; j < new_array.n_rows; j++) {
                    for(int i = 0; i < new_array.n_cols; i++) {
                        int count = 0;
                        count += self.is_valid(i - 1, j - 1) && vm->py_eq(self._get(i - 1, j - 1), value);
                        count += self.is_valid(i, j - 1) && vm->py_eq(self._get(i, j - 1), value);
                        count += self.is_valid(i + 1, j - 1) && vm->py_eq(self._get(i + 1, j - 1), value);
                        count += self.is_valid(i - 1, j) && vm->py_eq(self._get(i - 1, j), value);
                        count += self.is_valid(i + 1, j) && vm->py_eq(self._get(i + 1, j), value);
                        count += self.is_valid(i - 1, j + 1) && vm->py_eq(self._get(i - 1, j + 1), value);
                        count += self.is_valid(i, j + 1) && vm->py_eq(self._get(i, j + 1), value);
                        count += self.is_valid(i + 1, j + 1) && vm->py_eq(self._get(i + 1, j + 1), value);
                        new_array._set(i, j, VAR(count));
                    }
                }
            } else if(neighborhood == "von Neumann") {
                for(int j = 0; j < new_array.n_rows; j++) {
                    for(int i = 0; i < new_array.n_cols; i++) {
                        int count = 0;
                        count += self.is_valid(i, j - 1) && vm->py_eq(self._get(i, j - 1), value);
                        count += self.is_valid(i - 1, j) && vm->py_eq(self._get(i - 1, j), value);
                        count += self.is_valid(i + 1, j) && vm->py_eq(self._get(i + 1, j), value);
                        count += self.is_valid(i, j + 1) && vm->py_eq(self._get(i, j + 1), value);
                        new_array._set(i, j, VAR(count));
                    }
                }
            } else {
                vm->ValueError("neighborhood must be 'Moore' or 'von Neumann'");
            }
            return new_array_obj;
        });

        vm->bind_func(type, "count", 2, [](VM* vm, ArgsView args) {
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            PyVar value = args[1];
            int count = 0;
            for(int i = 0; i < self.numel; i++) {
                count += vm->py_eq(self.data[i], value);
            }
            return VAR(count);
        });

        vm->bind_func(type, "find_bounding_rect", 2, [](VM* vm, ArgsView args) {
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            PyVar value = args[1];
            int left = self.n_cols;
            int top = self.n_rows;
            int right = 0;
            int bottom = 0;
            for(int j = 0; j < self.n_rows; j++) {
                for(int i = 0; i < self.n_cols; i++) {
                    if(vm->py_eq(self._get(i, j), value)) {
                        left = (std::min)(left, i);
                        top = (std::min)(top, j);
                        right = (std::max)(right, i);
                        bottom = (std::max)(bottom, j);
                    }
                }
            }
            int width = right - left + 1;
            int height = bottom - top + 1;
            if(width <= 0 || height <= 0) {
                return vm->None;
            }
            Tuple t(4);
            t[0] = VAR(left);
            t[1] = VAR(top);
            t[2] = VAR(width);
            t[3] = VAR(height);
            return VAR(std::move(t));
        });
    }

    void _gc_mark(VM* vm) const {
        for(int i = 0; i < numel; i++) {
            vm->obj_gc_mark(data[i]);
        }
    }

    ~Array2d() { delete[] data; }
};

struct Array2dIter {
    PK_ALWAYS_PASS_BY_POINTER(Array2dIter)

    PyVar ref;
    Array2d* a;
    int i;

    Array2dIter(PyVar ref, Array2d* a) : ref(ref), a(a), i(0) {}

    void _gc_mark(VM* vm) const { vm->obj_gc_mark(ref); }

    static void _register(VM* vm, PyObject* mod, PyObject* type) {
        vm->bind__iter__(type->as<Type>(), [](VM* vm, PyVar _0) { return _0; });
        vm->bind__next__(type->as<Type>(), [](VM* vm, PyVar _0) -> unsigned {
            Array2dIter& self = PK_OBJ_GET(Array2dIter, _0);
            if(self.i == self.a->numel) {
                return 0;
            }
            std::div_t res = std::div(self.i, self.a->n_cols);
            vm->s_data.emplace(VM::tp_int, res.rem);
            vm->s_data.emplace(VM::tp_int, res.quot);
            vm->s_data.push(self.a->data[self.i++]);
            return 3;
        });
    }
};

void add_module_array2d(VM* vm) {
    PyObject* mod = vm->new_module("array2d");

    vm->register_user_class<Array2d>(mod, "array2d", VM::tp_object, true);
    vm->register_user_class<Array2dIter>(mod, "_array2d_iter");

    Type array2d_iter_t = vm->_tp_user<Array2d>();
    vm->bind__iter__(array2d_iter_t,
                     [](VM* vm, PyVar _0) { return vm->new_user_object<Array2dIter>(_0, &_0.obj_get<Array2d>()); });
    vm->_all_types[array2d_iter_t].op__iter__ = [](VM* vm, PyVar _0) {
        vm->new_stack_object<Array2dIter>(vm->_tp_user<Array2dIter>(), _0, &_0.obj_get<Array2d>());
    };
}

}  // namespace pkpy
