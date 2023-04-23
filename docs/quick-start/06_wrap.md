---
icon: code
label: 'Wrap a native struct'
order: 50
---

Add `PY_CLASS` macro into your `struct` and implement a static function `_register`.

```cpp
PY_CLASS(T, mod, name)
```

## Example

```cpp
#include "pocketpy.h"

using namespace pkpy;

struct Vector2 {
    PY_CLASS(Vector2, test, Vector2)
    float x;
    float y;

    Vector2(float x, float y) : x(x), y(y) {}

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_static_method<2>(type, "__new__", [](VM* vm, ArgsView args){
            float x = vm->num_to_float(args[0]);
            float y = vm->num_to_float(args[1]);
            return VAR_T(Vector2, x, y);
        });

        vm->bind_method<0>(type, "__repr__", [](VM* vm, ArgsView args){
            Vector2& self = CAST(Vector2&, args[0]);
            std::stringstream ss;
            ss << "Vector2(" << self.x << ", " << self.y << ")";
            return VAR(ss.str());
        });

        vm->bind_method<1>(type, "__add__", [](VM* vm, ArgsView args){
            Vector2& self = CAST(Vector2&, args[0]);
            Vector2& other = CAST(Vector2&, args[1]);
            return VAR_T(Vector2, self.x + other.x, self.y + other.y);
        });

        vm->bind_method<1>(type, "__sub__", [](VM* vm, ArgsView args){
            Vector2& self = CAST(Vector2&, args[0]);
            Vector2& other = CAST(Vector2&, args[1]);
            return VAR_T(Vector2, self.x - other.x, self.y - other.y);
        });

        vm->bind_method<1>(type, "__mul__", [](VM* vm, ArgsView args){
            Vector2& self = CAST(Vector2&, args[0]);
            f64 other = vm->num_to_float(args[1]);
            return VAR_T(Vector2, self.x * other, self.y * other);
        });

        vm->bind_method<1>(type, "__truediv__", [](VM* vm, ArgsView args){
            Vector2& self = CAST(Vector2&, args[0]);
            f64 other = vm->num_to_float(args[1]);
            return VAR_T(Vector2, self.x / other, self.y / other);
        });

        vm->bind_method<1>(type, "__eq__", [](VM* vm, ArgsView args){
            Vector2& self = CAST(Vector2&, args[0]);
            Vector2& other = CAST(Vector2&, args[1]);
            return VAR(self.x == other.x && self.y == other.y);
        });

        vm->bind_method<1>(type, "__ne__", [](VM* vm, ArgsView args){
            Vector2& self = CAST(Vector2&, args[0]);
            Vector2& other = CAST(Vector2&, args[1]);
            return VAR(self.x != other.x || self.y != other.y);
        });
    }
};

int main(){
    VM* vm = new VM(true);
    // create a new module `test`
    PyObject* mod = vm->new_module("test");
    // register `Vector2` to `test` module
    Vector2::register_class(vm, mod);
    return 0;
}
```

## Usage

```python
from test import Vector2

a = Vector2(1.0, 2.0)
b = Vector2(-1.0, -2.0)
print(a + b)    # Vector2(0.0, 0.0)
```