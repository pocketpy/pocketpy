---
icon: cpu
title: Write C Bindings
order: 18
---

In order to use a C/C++ library in python, you need to write bindings for it.

pkpy uses an universal signature to wrap a C function pointer as a python function or method, i.e `py_CFunction`.

```c
typedef bool (*py_CFunction)(int argc, py_Ref argv);
```
+ `argc` is the number of arguments passed to the function.
+ `argv` is the pointer to the first argument.

If successful, the function should return `true` and set the return value in `py_retval()`. In case there is no return value, you should use `py_newnone(py_retval())`.
If an error occurs, the function should raise an exception and return `false`.

## Steps

### Bind a simple function
Say you have a function `add` that takes two integers and returns their sum.
```c
int add(int a, int b) {
    return a + b;
}
```

Here is how you can write the binding for it:
```c
// 1. Define a wrapper function with the signature `py_CFunction`.
bool py_add(int argc, py_Ref argv) {
    // 2. Check the number of arguments.
    PY_CHECK_ARGC(2);
    // 3. Check the type of arguments.
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    // 4. Convert the arguments into C types.
    int _0 = py_toint(py_arg(0));
    int _1 = py_toint(py_arg(1));
    // 5. Call the original function.
    int res = add(_0, _1);
    // 6. Set the return value.
    py_newint(py_retval(), res);
    // 7. Return `true`.
    return true;
}
```

Once you have the wrapper function, you can bind it to a python module via `py_bindfunc`.
```c
py_GlobalRef mod = py_getmodule("__main__");
py_bindfunc(mod, "add", py_add);
```

Alternatively, you can use `py_bind` with a signature, which allows you to specify some default values.
```c
py_GlobalRef mod = py_getmodule("__main__");
py_bind(mod, "add(a, b=1)", py_add);
```

### Bind a struct
If you have a struct like this:
```c
typedef struct MyStruct{
    int x;
    int y;
}MyStruct;
```

Here's how you can create a `MyStruct`:
```c
// 1. Define the "new" function to create a MyStruct.
MyStruct* py_new_MyStruct(py_OutRef out, int x, int y) {
    // 2. Create a new object.
    MyStruct* res = py_newobject(out, tp_object, 2, sizeof(MyStruct));
    // 3. Put x and y into the object.
    res->x = x;
    res->y = y;
    return res;
}
// 4. Define a wrapper function with the signature `py_CFunction`.
bool MyStruct__new__(int argc, py_Ref argv) {
    // 5. Check the number of arguments.
    PY_CHECK_ARGC(3);
    // 6. Check the type of arguments.
    PY_CHECK_ARG_TYPE(0, tp_type);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    // 7. Convert the arguments into C types.
    int x = py_toint(py_arg(1));
    int y = py_toint(py_arg(2));
    // 8. Call the previous function to create MyStruct.
    MyStruct* res = py_new_MyStruct(py_pushtmp(), x, y);
    // 9. Set the created MyStruct into the return value register.
    py_assign(py_retval(), py_peek(-1));
    // 10. The result is already in the return value register, pop MyStruct is safe now.
    py_pop();
    // 11. Don't forget to return `true`.
    return true;
}
```

We also would like to have a function to get x from `MyStruct`:
```c
bool MyStruct_x(int argc, py_Ref argv) {
    // 1. Check the number of arguments.
    PY_CHECK_ARGC(1);
    // 2. Convert the arguments into C types.
    MyStruct* self = py_touserdata(argv);
    // 3. Set the x value.
    py_newint(py_retval(), self->x);
    // 4. Return `true`.
    return true;
}
```

Similar to the simple function binding, you can bind your struct to a python module:
```c
// 1. Put MyStruct into a new module `mystruct`
py_GlobalRef mod = py_newmodule("mystruct");
// 2. MyStruct is named as `custom_struct`
py_Type mystruct = py_newtype("custom_struct", tp_object, mod, NULL);
// 3. Bind functions.
py_bind(py_tpobject(mystruct), "__new__(cls, x: int, y: int)", MyStruct__new__);
py_bind(mod, "get_x(cls: mystruct)", MyStruct_x);
```

Now you can use MyStruct like this:
```python
import mystruct
test = mystruct.custom_struct(10, 100)
print(mystruct.get_x(test))
```

### Bind a function with arbitrary argument lists
Sometimes you want a function that takes arbitrary input arguments. For example, sum several numbers in the table, 
or make a simple `print` function.

#### Sum several numbers
Say you have 2,3,4,5,6 and put them into the `sum` function. Here's an implementation:
```c
bool py_sum(int argc, py_Ref argv) {
    // 1. These numbers are packed as a tuple
	PY_CHECK_ARG_TYPE(0, tp_tuple);
    // 2. Get the length of the tuple
	int len = py_tuple_len(py_arg(0));
	int res = 0;
    // 3. Sum the numbers up.
	for (int i = 0; i < len; i++) {
		int _0 = py_toint(py_tuple_getitem(py_arg(0), i));
		res += _0;
	}
    // 4. Set the result.
	py_newint(py_retval(), res);
    // 5. Return `true`.
	return res;
}
```

And then bind it:
```c
py_GlobalRef mod = py_newmodule("sumary");
py_bind(mod, "sum(*values: tuple[int])", py_sum);
```

It can be used like this:
```python
import sumary
print(sumary.sum(2,3,4,5,6))
```

#### Make a simple print function
Let's make a simple print function now. It takes arbitrary argument `*values`, and `end`/`sep`
is not necessary. It's so simple that only string argument is acceptable. 

Here's an implementation:
```c
bool py_print(int argc, py_Ref argv) {
    // 1. *values is always a tuple.
	PY_CHECK_ARG_TYPE(0, tp_tuple);
    // 2. Get the length of tuple.
	int len = py_tuple_len(py_arg(0));
	const char* end = "\n";
	const char* sep = " ";
    // 3. First arg is sep, but it could be None. 
	if (!py_isnone(py_arg(1))) {
		PY_CHECK_ARG_TYPE(1, tp_str);
		sep = py_tostr(py_arg(1));
	}
    // 4. Second arg is end, it also can be None.
	if (!py_isnone(py_arg(2))) {
		PY_CHECK_ARG_TYPE(2, tp_str);
		end = py_tostr(py_arg(2));
	}
    // 5. Print.
	for (int i = 0; i < len; i++) {
		if (i > 0) {
			printf("%s", sep);
		}
        // 6. It can print iterable like `list` if you modify this line.
		printf("%s", py_tostr(py_tuple_getitem(py_arg(0), i)));
	}
	printf("%s", end);
    // 7. All the functions should return a value, here None is returned.
	py_newnone(py_retval());
	return true;
}
```

And then bind:
```c
py_GlobalRef mod = py_newmodule("myprint");
py_bind(mod, "my_print(*values: object, sep: str | None = None, end: str | None = None)", py_print);
```

It can print names like this:
```python
import myprint
myprint.my_print('Bob','Mary', end = 'Cake', sep = '|')
```


See also:
+ [`py_bind`](/c-api/functions/#py_bind)
+ [`py_bindmethod`](/c-api/functions/#py_bindmethod)
+ [`py_bindfunc`](/c-api/functions/#py_bindfunc)
+ [`py_bindproperty`](/c-api/functions/#py_bindproperty)
+ [`py_newmodule`](/c-api/functions/#py_newmodule)
+ [`py_newtype`](/c-api/functions/#py_newtype)