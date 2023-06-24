---
label: Virtual machine
icon: dot
order: 20
---

The `VM` class provides a sandboxed Python environment and a set of APIs for interacting with it.
Using the namespace `PocketPython` before any operations.

```csharp
using PocketPython;
```

### Construction

+ `VM()`

    Create a new Python virtual machine.

### Code Execution

+ `CodeObject Compile(string source, string filename, CompileMode mode)`

    Compile Python source code into a `CodeObject` that can be executed later.
    The `filename` parameter is used for error reporting, you can set it to `main.py` if you don't need it.
    The `mode` parameter specifies the compile mode, see [CompileMode](../quick-start/exec/#compile-mode) for details.

+ `object Exec(CodeObject co, PyModule mod = null)`

    Execute a `CodeObject` in the given module.
    The `mod` parameter specifies the module in which the code will be executed.
    If it is `null`, the code will be executed in the main module.

+ `object Exec(string source, string filename, CompileMode mode = CompileMode.EXEC_MODE, PyModule mod = null)`

    Compile and execute Python source code in the given module. It is equivalent to `Exec(Compile(source, filename, mode), mod)`.

+ `object Eval(string source, PyModule mod = null)`

    Evaluate an expression in the given module.

+ `object Call(object callable, object[] args, Dictionary<string, object> kwargs)`

    Call a Python callable object with the given arguments and keyword arguments. It is equivalent to `callable(*args, **kwargs)` in Python.

+ `object CallMethod(object obj, string name, params object[] args)`

    Call a method of a Python object with the given arguments. It is equivalent to `obj.name(*args)` in Python.


### Attribute Access

+ `object GetAttr(object obj, string name, bool throwErr = true)`

    Get an attribute of a Python object. It is equivalent to `obj.name` in Python.
    If `throwErr` is `true`, it will throw an exception if the attribute does not exist.
    Otherwise, it will return `null`.

+ `NoneType SetAttr(object obj, string name, object value)`

    Set an attribute of a Python object. It is equivalent to `obj.name = value` in Python.

+ `bool HasAttr(object obj, string name)`

    Check if a Python object has the given attribute. It is equivalent to `hasattr(obj, name)` in Python.

### Module Access

+ `Dictionary<string, PyModule> modules`

    A dictionary that maps module names to `PyModule` objects.
    You can use it to access the modules that have been imported.

+ `Dictionary<string, string> lazyModules`

    A dictionary stores all unimported modules. You can add Python source into this dictionary.
    It will be initialized and moved to `modules` when it is first imported.

+ `PyModule NewModule(string name)`

    Create a new module with the given name at runtime. The module will be added to `modules` automatically.

+ `PyModule PyImport(string name)`

    Import a Python module. It is equivalent to `import name` in Python. It first checks if the module has been imported, if not, it will try to load the module from `lazyModules`.


### Type Conversion

+ `T PyCast<T>(object obj)`

    Convert a Python object to a C# object. It is equivalent to `obj as T` in C#.
    Raise `TypeError` if the conversion fails.

+ `bool IsInstance(object obj, PyTypeObject type)`

    Check if a Python object is an instance of the given type. It is equivalent to `isinstance(obj, type)` in Python.

+ `void CheckType<T>(object t)`

    Check if `t is T`. Raise `TypeError` if the check fails.

+ `bool PyEquals(object lhs, object rhs)`

    Check if two Python objects are equal. It is equivalent to `lhs == rhs` in Python. This is different from `==` or `object.ReferenceEquals` in C#. You should always use this method to compare Python objects.

+ `object PyIter(object obj)`

    Get an iterator of a Python object. It is equivalent to `iter(obj)` in Python.

+ `object PyNext(object obj)`

    Get the next element of a Python iterator. It is equivalent to `next(obj)` in Python.

+ `bool PyBool(object obj)`

    Convert a Python object to a boolean value. It is equivalent to `bool(obj)` in Python.

+ `string PyStr(object obj)`

    Convert a Python object to a string. It is equivalent to `str(obj)` in Python.

+ `string PyRepr(object obj)`

    Convert a Python object to a string representation. It is equivalent to `repr(obj)` in Python.

+ `int PyHash(object obj)`

    Get the hash value of a Python object. It is equivalent to `hash(obj)` in Python.

+ `List<object> PyList(object obj)`

    Convert an `Iterable` Python object to a list. It is equivalent to `list(obj)` in Python.

### Callbacks

+ `System.Action<string> stdout = Debug.Log`

    A callback that will be called when the Python code invokes `print` function.
    By default, it will print the message to Unity console.

+ `System.Action<string> stderr = null`

    A callback that will be called when the Python code emits an error message.
    By default, an Exception will be raised.
    You can set it to `Debug.LogError` for printing to the Unity Console.

### Debug Flag

+ `bool debug = false`

    A flag that controls whether to print debug messages to Unity console.
    You can set it to `true` to enable debug messages, or `false` to disable them.
