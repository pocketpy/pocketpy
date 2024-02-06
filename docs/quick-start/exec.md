---
icon: dot
label: 'Execute Python Code'
order: 93
---

### Simple execution

Once you have a `VM` instance, you can execute python code by calling `exec` method.

#### `PyObject* exec(Str source, Str filename, CompileMode mode, PyObject* _module=nullptr)`

+ `source`, the python source code to be executed
+ `filename`, the filename of the source code. This is used for error reporting
+ `mode`, the compile mode. See below for details
+ `module`, the module where the code will be executed. If `nullptr`, the code will be executed in the `__main__` module

`exec` handles possible exceptions and returns a `PyObject*`.
If the execution is not successful, e.g. a syntax error or a runtime exception,
the return value will be `nullptr`.

There are also overloaded versions of `exec` and `eval`, which is useful for simple execution:
+ `PyObject* exec(Str source)`
+ `PyObject* eval(Str source)`

### Compile mode

The `mode` parameter controls how the source code is compiled. There are 5 possible values:
+ `EXEC_MODE`, this is the default mode. Just do normal execution.
+ `EVAL_MODE`, this mode is used for evaluating a single expression. The `source` should be a single expression. It cannot contain any statements.
+ `REPL_MODE`, this mode is used for REPL. It is similar to `EXEC_MODE`, but generates `PRINT_EXPR` opcode when necessary.
+ `CELL_MODE`, this mode is designed for Jupyter like execution. It is similar to `EXEC_MODE`, but generates `PRINT_EXPR` opcode when necessary.
+ `JSON_MODE`, this mode is used for JSON parsing. It is similar to `EVAL_MODE`, but uses a lexing rule designed for JSON.


### Fine-grained execution

In some cases, you may want to execute python code in a more fine-grained way.
These two methods are provided for this purpose:

+ `CodeObject_ compile(Str source, Str filename, CompileMode mode, bool unknown_global_scope)`
+ `PyObject* _exec(CodeObject_ co, PyObject* _module)`

1. `compile` compiles the source code into a `CodeObject_` instance. Leave `unknown_global_scope` to `false` if you don't know what it means.
2. `_exec` executes the `CodeObject_` instance.

!!!
`_exec` does not handle exceptions, you need to use `try..catch` manually.
!!!

```cpp
try{
    CodeObject_ code = vm->compile("a[0]", "main.py", EXEC_MODE, false);
    vm->_exec(code, vm->_main);
}catch(Exception& e){
    // use e.summary() to get a summary of the exception
    std::cerr << e.summary() << std::endl;
}
```
