---
icon: book
order: -5
label: Coding Style Guide
---

# Coding Style Guide


## For Python

Use [PEP-8](https://www.python.org/dev/peps/pep-0008/) as the coding style guide.

## For C++

### Naming rules

For class names, always use **PascalCase**

```cpp
// Correct
class FooBar {};

// Wrong
class fooBar {};
class foo_bar {};
```

For function and methods, use **snake_case**

```cpp
// Correct
int test_func(int x) { return x+1; }

// Wrong
int TestFunc(int x) { return x+1; }
int testFunc(int x) { return x+1; }
```

For special python objects, use the same name as in python.

```cpp
auto x = vm->None;
vm->SyntaxError(...);
vm->TypeError(...);
vm->call(obj, __repr__);
```

For global constants, use **k** prefix with **PascalCase**

```cpp
const int kMaxCount = 10;
const float kMinValue = 1.0;
```

For macros, use **SNAKE_CASE**

```cpp
#define FOO_BAR 1
#define TEST(x) x+1
```

### Access control

Please use python style access control.

We do not recommend to use C++ keywords such as `private` or `public` to achieve access control. Also do not write any trivial setter/getter.

Use a single `_` as prefix to indicate a function or variable is for internal use.

```cpp
class FooBar {
public:
    int _count;
    int inc() { _count+=1; }
    void _clear() { _count=0; }
}
```

`_` prefix is just a warning to remind you to use such members carefully.

It does not forbid users to access internal members.

### Use compact style

Try to make the code compact if it does not affect readability.

```cpp
// Correct
if(x == 1) break;

// Wrong
if(x == 1){
	break;
}
```

### For `std::shared_ptr<T>`

Use a `_` suffix to indicate a type is a shared pointer.

```cpp
using CodeObject_ = std::shared_ptr<CodeObject>;
CodeObject_ co = std::make_shared<CodeObject>();
```

