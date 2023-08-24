---
icon: log
title: 'Cheat Sheet'
order: 22
---

## Basics

Setup pocketpy

```cpp
#include "pocketpy.h"
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
std::cout << CAST(i64, obj);  // 123
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
obj = VAR(1);			// create a int
obj = VAR(1.0);		// create a float
obj = VAR("123");	// create a string
obj = VAR(true);	// create a bool
```

Create a tuple object

```cpp
// obj = (1, 1.0, '123')
Tuple t(3);
t[0] = VAR(1);
t[1] = VAR(1.0);
t[2] = VAR("123");
PyObject* obj = VAR(std::move(t));
```

Create a list object

```cpp
// obj = [1, 1.0, '123']
List t;
t.push_back(VAR(1));
t.push_bask(VAR(1.0));
t.push_back(VAR("123"));
PyObject* obj = VAR(std::move(t));
```

Create a dict object

```cpp
// obj = {'x': 1, 'y': '123'}
Dict d(vm);
d.set(VAR('x'), VAR(1));
d.set(VAR('y'), VAR('123'));
PyObject* obj = VAR(std::move(d));
```

Get native types from python objects

```cpp
PyObject* obj;
i64 a = CAST(i64, obj);
f64 b = CAST(f64, obj);
Str& c = CAST(Str&, obj);			// reference cast
bool d = CAST(bool, obj);

Tuple& e = CAST(Tuple&, obj);	// reference cast
List& f = CAST(List&, obj);		// reference cast
Dict& g = CAST(Dict&, obj);		// reference cast
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
PyObject* voidp_t = VoidP::_type(vm);
```

## Access attributes

Check an object supports attribute access

```cpp
PyObject* obj;
bool ok = !is_tagged(obj) && obj->is_attr_valid();
```

Get and set attributes

```python
class MyClass:
  def __init__(self, x, y):
    self.x = x
    self.y = y
```

```cpp
PyObject* obj = vm->exec("MyClass(1, 2)");
PyObject* x = vm->getattr(obj, "x");	// obj.x
vm->setattr(obj, "x", VAR(3));				// obj.x = 3
```

## Call python functions

```python
def add(a, b):
  return a + b 
```

```cpp
PyObject* f_add = vm->eval("add");
PyObject* ret = vm->call(f_add, VAR(1), VAR(2));
std::cout << CAST(i64, ret);	// 3
```