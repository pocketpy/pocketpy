#include "pocketpy/array2d.h"

namespace pkpy{

struct Array2d{
    PK_ALWAYS_PASS_BY_POINTER(Array2d)
    PY_CLASS(Array2d, array2d, array2d)

    PyObject** data;
    int n_cols;
    int n_rows;
    int numel;

    Array2d(){
        data = nullptr;
        n_cols = 0;
        n_rows = 0;
        numel = 0;
    }

    Array2d* _() { return this; }

    void init(int n_cols, int n_rows){
        this->n_cols = n_cols;
        this->n_rows = n_rows;
        this->numel = n_cols * n_rows;
        this->data = new PyObject*[numel];
    }

    bool is_valid(int col, int row) const{
        return 0 <= col && col < n_cols && 0 <= row && row < n_rows;
    }

    PyObject* _get(int col, int row){
        return data[row * n_cols + col];
    }

    void _set(int col, int row, PyObject* value){
        data[row * n_cols + col] = value;
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind(type, "__new__(cls, *args, **kwargs)", [](VM* vm, ArgsView args){
            Type cls = PK_OBJ_GET(Type, args[0]);
            return vm->heap.gcnew<Array2d>(cls);
        });

        vm->bind(type, "__init__(self, n_cols: int, n_rows: int, default=None)", [](VM* vm, ArgsView args){
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            int n_cols = CAST(int, args[1]);
            int n_rows = CAST(int, args[2]);
            if(n_cols <= 0 || n_rows <= 0){
                vm->ValueError("n_cols and n_rows must be positive integers");
            }
            self.init(n_cols, n_rows);
            if(vm->py_callable(args[3])){
                for(int i = 0; i < self.numel; i++) self.data[i] = vm->call(args[3]);
            }else{
                for(int i = 0; i < self.numel; i++) self.data[i] = args[3];
            }
            return vm->None;
        });

        PY_READONLY_FIELD(Array2d, "n_cols", _, n_cols);
        PY_READONLY_FIELD(Array2d, "n_rows", _, n_rows);
        PY_READONLY_FIELD(Array2d, "width", _, n_cols);
        PY_READONLY_FIELD(Array2d, "height", _, n_rows);
        PY_READONLY_FIELD(Array2d, "numel", _, numel);

        vm->bind(type, "is_valid(self, col: int, row: int)", [](VM* vm, ArgsView args){
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            int col = CAST(int, args[1]);
            int row = CAST(int, args[2]);
            return VAR(self.is_valid(col, row));
        });

        vm->bind(type, "get(self, col: int, row: int, default=None)", [](VM* vm, ArgsView args){
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            int col = CAST(int, args[1]);
            int row = CAST(int, args[2]);
            if(!self.is_valid(col, row)) return args[3];
            return self._get(col, row);
        });

        vm->bind__getitem__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){
            Array2d& self = PK_OBJ_GET(Array2d, _0);
            const Tuple& xy = CAST(Tuple&, _1);
            int col = CAST(int, xy[0]);
            int row = CAST(int, xy[1]);
            if(!self.is_valid(col, row)){
                vm->IndexError(_S('(', col, ", ", row, ')', " is not a valid index for array2d(", self.n_cols, ", ", self.n_rows, ')'));
            }
            return self._get(col, row);
        });

        vm->bind__setitem__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1, PyObject* _2){
            Array2d& self = PK_OBJ_GET(Array2d, _0);
            const Tuple& xy = CAST(Tuple&, _1);
            int col = CAST(int, xy[0]);
            int row = CAST(int, xy[1]);
            if(!self.is_valid(col, row)){
                vm->IndexError(_S('(', col, ", ", row, ')', " is not a valid index for array2d(", self.n_cols, ", ", self.n_rows, ')'));
            }
            self._set(col, row, _2);
        });

        vm->bind__iter__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0){
            Array2d& self = PK_OBJ_GET(Array2d, _0);
            List t(self.n_rows);
            List row(self.n_cols);
            for(int j = 0; j < self.n_rows; j++){
                for(int i = 0; i < self.n_cols; i++) row[i] = self._get(i, j);
                t[j] = VAR(row);    // copy
            }
            return vm->py_iter(VAR(std::move(t)));
        });

        vm->bind__len__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0){
            Array2d& self = PK_OBJ_GET(Array2d, _0);
            return (i64)self.n_rows;
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0){
            Array2d& self = PK_OBJ_GET(Array2d, _0);
            return VAR(_S("array2d(", self.n_cols, ", ", self.n_rows, ')'));
        });

        vm->bind(type, "map(self, f)", [](VM* vm, ArgsView args){
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            PyObject* f = args[1];
            PyObject* new_array_obj = vm->heap.gcnew<Array2d>(Array2d::_type(vm));
            Array2d& new_array = PK_OBJ_GET(Array2d, new_array_obj);
            new_array.init(self.n_cols, self.n_rows);
            for(int i = 0; i < new_array.numel; i++){
                new_array.data[i] = vm->call(f, self.data[i]);
            }
            return new_array_obj;
        });

        vm->bind(type, "copy(self)", [](VM* vm, ArgsView args){
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            PyObject* new_array_obj = vm->heap.gcnew<Array2d>(Array2d::_type(vm));
            Array2d& new_array = PK_OBJ_GET(Array2d, new_array_obj);
            new_array.init(self.n_cols, self.n_rows);
            for(int i = 0; i < new_array.numel; i++){
                new_array.data[i] = self.data[i];
            }
            return new_array_obj;
        });

        vm->bind(type, "fill_(self, value)", [](VM* vm, ArgsView args){
            Array2d& self = PK_OBJ_GET(Array2d, args[0]); 
            for(int i = 0; i < self.numel; i++){
                self.data[i] = args[1];
            }
            return vm->None;
        });

        vm->bind(type, "apply_(self, f)", [](VM* vm, ArgsView args){
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            PyObject* f = args[1];
            for(int i = 0; i < self.numel; i++){
                self.data[i] = vm->call(f, self.data[i]);
            }
            return vm->None;
        });

        vm->bind(type, "copy_(self, other)", [](VM* vm, ArgsView args){
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            if(is_non_tagged_type(args[1], VM::tp_list)){
                const List& list = PK_OBJ_GET(List, args[1]);
                if(list.size() != self.numel){
                    vm->ValueError("list size must be equal to the number of elements in the array2d");
                }
                for(int i = 0; i < self.numel; i++){
                    self.data[i] = list[i];
                }
                return vm->None;
            }
            Array2d& other = CAST(Array2d&, args[1]);
            // if self and other have different sizes, re-initialize self
            if(self.n_cols != other.n_cols || self.n_rows != other.n_rows){
                delete self.data;
                self.init(other.n_cols, other.n_rows);
            }
            for(int i = 0; i < self.numel; i++){
                self.data[i] = other.data[i];
            }
            return vm->None;
        });

        vm->bind__eq__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){
            Array2d& self = PK_OBJ_GET(Array2d, _0);
            if(!is_non_tagged_type(_1, Array2d::_type(vm))) return vm->NotImplemented;
            Array2d& other = PK_OBJ_GET(Array2d, _1);
            if(self.n_cols != other.n_cols || self.n_rows != other.n_rows) return vm->False;
            for(int i = 0; i < self.numel; i++){
                if(vm->py_ne(self.data[i], other.data[i])) return vm->False;
            }
            return vm->True;
        });

        // for cellular automata
        vm->bind(type, "count_neighbors(self, value) -> array2d[int]", [](VM* vm, ArgsView args){
            Array2d& self = PK_OBJ_GET(Array2d, args[0]);
            PyObject* new_array_obj = vm->heap.gcnew<Array2d>(Array2d::_type(vm));
            Array2d& new_array = PK_OBJ_GET(Array2d, new_array_obj);
            new_array.init(self.n_cols, self.n_rows);
            PyObject* value = args[1];
            for(int j = 0; j < new_array.n_rows; j++){
                for(int i = 0; i < new_array.n_cols; i++){
                    int count = 0;
                    count += self.is_valid(i-1, j-1) && vm->py_eq(self._get(i-1, j-1), value);
                    count += self.is_valid(i, j-1) && vm->py_eq(self._get(i, j-1), value);
                    count += self.is_valid(i+1, j-1) && vm->py_eq(self._get(i+1, j-1), value);
                    count += self.is_valid(i-1, j) && vm->py_eq(self._get(i-1, j), value);
                    count += self.is_valid(i+1, j) && vm->py_eq(self._get(i+1, j), value);
                    count += self.is_valid(i-1, j+1) && vm->py_eq(self._get(i-1, j+1), value);
                    count += self.is_valid(i, j+1) && vm->py_eq(self._get(i, j+1), value);
                    count += self.is_valid(i+1, j+1) && vm->py_eq(self._get(i+1, j+1), value);
                    new_array._set(i, j, VAR(count));
                }
            }
            return new_array_obj; 
        });
    }

    void _gc_mark() const{
        for(int i = 0; i < numel; i++) PK_OBJ_MARK(data[i]);
    }

    ~Array2d(){
        delete[] data;
    }
};

void add_module_array2d(VM* vm){
    PyObject* mod = vm->new_module("array2d");

    Array2d::register_class(vm, mod);
}


}   // namespace pkpy