---
icon: dot
label: 'Create modules'
order: 50
---

Modules are stored in `vm->_modules` and `vm->_lazy_modules`.
They are both dict-like objects.

### Lazy modules

A lazy module is a python source file.
It is compiled and executed when it is imported.
Use `[]` operator to add a lazy module.

```cpp
vm->_lazy_modules["test"] = "pi = 3.14";
```

```python
import test
print(test.pi)  # 3.14
```

### Native modules

A native module is a module written in c++ or mixed c++/python.
Native modules are always compiled and executed when the VM is created.

To creata a native module,
use `vm->new_module(Str name)`.

```cpp
PyObject* mod = vm->new_module("test");
mod->attr().set("pi", py_var(vm, 3.14));

vm->bind(mod, "add(a: int, b: int)",
    [](VM* vm, ArgsView args){
        int a = py_cast<int>(vm, args[0]);
        int b = py_cast<int>(vm, args[1]);
        return py_var(vm, a + b);
    });
```

```python
import test
print(test.pi)  # 3.14
print(test.add(1, 2))  # 3
```

### Module resolution order

When you do `import` a module, the VM will try to find it in the following order:

1. Search `vm->_modules`, if found, return it.
2. Search `vm->_lazy_modules`, if found, compile and execute it, then return it.
3. Try `vm->_import_handler`.


### Customized import handler

You can use `vm->_import_handler` to provide a custom import handler for the 3rd step.
if both `enable_os` and `PK_ENABLE_OS` are `true`, the default `import_handler` is as follows:

```cpp
inline Bytes _default_import_handler(const Str& name){
    std::filesystem::path path(name.sv());
    bool exists = std::filesystem::exists(path);
    if(!exists) return Bytes();
    std::string cname = name.str();
    FILE* fp = fopen(cname.c_str(), "rb");
    if(!fp) return Bytes();
    fseek(fp, 0, SEEK_END);
    std::vector<char> buffer(ftell(fp));
    fseek(fp, 0, SEEK_SET);
    fread(buffer.data(), 1, buffer.size(), fp);
    fclose(fp);
    return Bytes(std::move(buffer));
};
```

### Import module via cpp

You can use `vm->py_import` to import a module.
This is equivalent to `import` in python.
Return the module object if success.
