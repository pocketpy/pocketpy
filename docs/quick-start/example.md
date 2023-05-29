---
icon: dot
label: 'Complete example'
order: 0
---

In this example, we will create a `linalg` module
and implement a `vec2` type with some methods.
And make them available in python just like this.

```python
from linalg import vec2

# construct a vec2
a = vec2(1.0, 2.0)
b = vec2(0.0, -1.0)

# add two vec2
print(a + b)    # vec2(1.0, 1.0)

# set x component
a.x = 8.0
print(a)        # vec2(8.0, 2.0)

# use dot method
print(a.dot(b)) # -2.0
```

### Implement `Vec2` struct in cpp

```cpp
struct Vec2{
    float x, y;
    Vec2() : x(0.0f), y(0.0f) {}
    Vec2(float x, float y) : x(x), y(y) {}
    Vec2(const Vec2& v) : x(v.x), y(v.y) {}
    Vec2 operator+(const Vec2& v) const { return Vec2(x + v.x, y + v.y); }
    float dot(const Vec2& v) const { return x * v.x + y * v.y; }
};
```

### Create `PyVec2` wrapper

```cpp
struct PyVec2: Vec2 {
    PY_CLASS(PyVec2, linalg, vec2)

    PyVec2() : Vec2() {}
    PyVec2(const Vec2& v) : Vec2(v) {}
    PyVec2(const PyVec2& v) : Vec2(v) {}

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<3>(type, [](VM* vm, ArgsView args){
            float x = CAST_F(args[1]);
            float y = CAST_F(args[2]);
            return VAR(Vec2(x, y));
        });

        vm->bind__repr__(OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            PyVec2& self = _CAST(PyVec2&, obj);
            std::stringstream ss;
            ss << "vec2(" << self.x << ", " << self.y << ")";
            return VAR(ss.str());
        });

        vm->bind__add__(OBJ_GET(Type, type), [](VM* vm, PyObject* obj, PyObject* other){
            PyVec2& self = _CAST(PyVec2&, obj);
            PyVec2& other_ = CAST(PyVec2&, other);
            return VAR_T(PyVec2, self + other_);
        });

        vm->bind_method<1>(type, "dot", [](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            PyVec2& other = CAST(PyVec2&, args[1]);
            return VAR(self.dot(other));
        });
    }
};
```

### Create `linalg` module

```cpp
void add_module_linalg(VM* vm){
    PyObject* linalg = vm->new_module("linalg");
    // register PyVec2
    PyVec2::register_class(vm, linalg);
}
```

### Further reading

See [linalg.h](https://github.com/blueloveTH/pocketpy/blob/main/src/linalg.h) for the complete implementation.