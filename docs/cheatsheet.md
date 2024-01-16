---
icon: log
title: 'Cheatsheet'
order: 22
---

## Basics

Setup pocketpy

```cpp
#include "pocketpy.h"
using namespace pkpy;
```

Create a python virtual machine

```cpp
VM* vm = new VM();
```

Dispose a python virtual machine

```cpp
delete vm;
```

Execute a source string

```cpp
vm->exec("print('Hello!')");
```

Evaluate a source string

```cpp
PyObject* obj = vm->eval("123");
std::cout << py_cast<int>(vm, obj);  // 123
```

Compile a source string into a code object

```cpp
CodeObject_ co = vm->compile("print('Hello!')", "main.py", EXEC_MODE);
```

Execute a compiled code object

```cpp
try{
    vm->_exec(co);    // may throw
}catch(Exception& e){
    std::cerr << e.summary() << std::endl;
}
```

## Interop with native types

Create primitive objects

```cpp
PyObject* obj;
obj = py_var(vm, 1);		// create a int
obj = py_var(vm, 1.0);		// create a float
obj = py_var(vm, "123");	// create a string
obj = py_var(vm, true); 	// create a bool
```

Create a tuple object

```cpp
// obj = (1, 1.0, '123')
Tuple t(3);
t[0] = py_var(vm, 1);
t[1] = py_var(vm, 1.0);
t[2] = py_var(vm, "123");
PyObject* obj = py_var(vm, std::move(t));
```

Create a list object

```cpp
// obj = [1, 1.0, '123']
List t;
t.push_back(py_var(vm, 1));
t.push_back(py_var(vm, 1.0));
t.push_back(py_var(vm, "123"));
PyObject* obj = py_var(vm, std::move(t));
```

Create a dict object

```cpp
// obj = {'x': 1, 'y': '123'}
Dict d(vm);
d.set(py_var(vm, "x"), py_var(vm, 1));
d.set(py_var(vm, "y"), py_var(vm, "123"));
PyObject* obj = py_var(vm, std::move(d));
```

Get native types from python objects

```cpp
PyObject* obj;
i64 a = py_cast<i64>(vm, obj);
f64 b = py_cast<f64>(vm, obj);
Str& c = py_cast<Str&>(vm, obj);    // reference cast
bool d = py_cast<bool>(vm, obj);

Tuple& e = py_cast<Tuple&>(vm, obj);    // reference cast
List& f = py_cast<List&>(vm, obj);      // reference cast
Dict& g = py_cast<Dict&>(vm, obj);      // reference cast
```

Get native types without type checking

```cpp
// unsafe version 1 (for int and float you must use `_py_cast`)
i64 a = _py_cast<i64>(vm, obj);
f64 b = _py_cast<f64>(vm, obj);
Tuple& c = _py_cast<Tuple&>(vm, obj);
// unsafe version 2 (for others, you can use both versions)
Str& a_ = PK_OBJ_GET(Str, obj);
List& b_ = PK_OBJ_GET(List, obj);
Tuple& c_ = PK_OBJ_GET(Tuple, obj);
```

## Access python types

Access built-in python types

```cpp
PyObject* int_t = vm->_t(vm->tp_int);
PyObject* float_t = vm->_t(vm->tp_float);
PyObject* object_t = vm->_t(vm->tp_object);
PyObject* tuple_t = vm->_t(vm->tp_tuple);
PyObject* list_t = vm->_t(vm->tp_list);
```

Access extended python types

```cpp
// VoidP was defined by `PY_CLASS` macro
PyObject* voidp_t = VoidP::_type(vm);
```

Check if an object is a python type

```cpp
PyObject* obj;
bool ok = is_type(obj, vm->tp_int); // check if obj is an int
```

Get the type of a python object

```cpp
PyObject* obj = py_var(vm, 1);
PyObject* t = vm->_t(obj);      // <class 'int'>
```

Convert a type object into a type index

```cpp
PyObject* int_t = vm->_t(vm->tp_int);
Type t = PK_OBJ_GET(Type, int_t);
// t == vm->tp_int
```

## Access attributes

Check an object supports attribute access

```cpp
PyObject* obj;
bool ok = !is_tagged(obj) && obj->is_attr_valid();
```

```python
class MyClass:
  def __init__(self, x, y):
    self.x = x
    self.y = y

  def sum(self):
    return self.x + self.y
```

Get and set attributes

```cpp
PyObject* obj = vm->exec("MyClass(1, 2)");
PyObject* x = vm->getattr(obj, "x");	// obj.x
vm->setattr(obj, "x", py_var(vm, 3));	// obj.x = 3
```

## Call python functions

```python
def add(a, b):
  return a + b 
```

Call a function

```cpp
PyObject* f_add = vm->eval("add");
PyObject* ret = vm->call(f_add, py_var(vm, 1), py_var(vm, 2));
std::cout << py_cast<int>(vm, ret);	// 3
```

Call a method

```cpp
PyObject* obj = vm->exec("MyClass(1, 2)");
PyObject* ret = vm->call_method(obj, "sum");
std::cout << CAST(i64, ret);    // 3
```

Cache the name of a function or method to avoid string-based lookup

```cpp
// cache the name "add" to avoid string-based lookup
const static StrName m_sum("sum");
PyObject* ret = vm->call_method(obj, m_sum);
```

## Special operations

Compare two python objects

```cpp
PyObject* obj1 = py_var(vm, 1);
PyObject* obj2 = py_var(vm, 2);
bool ok = vm->py_eq(obj1, obj2);
```

Convert a python object to string

```cpp
PyObject* obj = py_var(vm, 123);
PyObject* s = vm->py_str(obj);  // 123
```

Get the string representation of a python object

```cpp
PyObject* obj = py_var(vm, "123");
std::cout << vm->py_repr(obj);  // '123'
```

Get the JSON representation of a python object

```cpp
PyObject* obj = py_var(vm, 123);
std::cout << vm->py_json(obj);  // "123"
```

Get the hash value of a python object

```cpp
PyObject* obj = py_var(vm, 1);
i64 h = vm->py_hash(obj);       // 1
```

Get the iterator of a python object

```cpp
PyObject* obj = vm->eval("range(3)");
PyObject* iter = vm->py_iter(obj);
```

Get the next item of an iterator

```cpp
PyObject* obj = vm->py_next(iter);
if(obj == vm->StopIteration){
    // end of iteration
}
```

Convert a python iterable to a list
  
```cpp
PyObject* obj = vm->eval("range(3)");
PyObject* list = vm->py_list(obj);
```

## Bindings

Bind a native function

```cpp
vm->bind(obj, "add(a: int, b: int) -> int", [](VM* vm, ArgsView args){
    int a = py_cast<int>(vm, args[0]);
    int b = py_cast<int>(vm, args[1]);
    return py_var(vm, a + b);
});

// Bind a native function with docstring

vm->bind(obj,
    "add(a: int, b: int) -> int",
    "add two integers", [](VM* vm, ArgsView args){
    int a = py_cast<int>(vm, args[0]);
    int b = py_cast<int>(vm, args[1]);
    return py_var(vm, a + b);
});
```

Bind a property

```cpp
    // getter and setter of property `x`
    vm->bind_property(type, "x: int",
      [](VM* vm, ArgsView args){
          Point& self = PK_OBJ_GET(Point, args[0]);
          return VAR(self.x);
      },
      [](VM* vm, ArgsView args){
          Point& self = PK_OBJ_GET(Point, args[0]);
          self.x = py_cast<int>(vm, args[1]);
          return vm->None;
      });
```

## Modules

Create a source module

```cpp
vm->_lazy_modules["test"] = "pi = 3.14";
// import test
// print(test.pi) # 3.14
```

Create a native module

```cpp
PyObject* mod = vm->new_module("test");
vm->setattr(mod, "pi", py_var(vm, 3.14));
```

