---
label: Bindings
icon: dot
order: 10
---

Bindings are methods and variables that are defined in C# and can be accessed from Python.
We provide two types of bindings: static bindings and dynamic bindings.

## Static Bindings

Static bindings wrap a C# class or struct and expose its methods and variables to Python.
This is the most common way to define bindings.
Static bindings are initialized at compile time.

### Manual Static Bindings

Manual static bindings directly create a Python equivalent of `def f(a, b, *args)` in C#.
To use it, you need to create a class that inherits from `PyTypeObject`.
And implement some abstract methods to specify the name and type of the Python type.
For example, to make `UnityEngine.Vector2` available in Python, you can write a `PyVector2Type`
class like the following.

```csharp
public class PyVector2Type: PyTypeObject{
    // The name of the type in Python
    public override string Name => "Vector2";

    // The corresponding C# type
    public override System.Type CSType => typeof(Vector2);
}
```

Next, you need to define each method and variable to be exposed to Python,
by using `[PythonBinding]` attribute.

!!!
We assume that you have necessary knowledge about
[Python's data model](https://docs.python.org/3/reference/datamodel.html).
Such as magic methods, `__new__`, `__init__`, `__add__` and so on.
Otherwise, you may have trouble understanding the following code.
!!!

Let's define a magic method `__add__`, it is used to implement the `+` operator in Python.
With `__add__`, `Vector2` object in Python can be added with another `Vector2` object.

```csharp
public class PyVector2Type: PyTypeObject{
    public override string Name => "Vector2";
    public override System.Type CSType => typeof(Vector2);

    [PythonBinding]
    public object __add__(Vector2 self, object other){
        // If the other object is not a Vector2, return NotImplemented
        if(!(other is Vector2)) return VM.NotImplemented;
        // Otherwise, return the result of addition
        return self + (Vector2)other;
    }
}
```

This is easy to understand.
Let's see another example, `__mul__`, it is used to implement the `*` operator in Python.
`Vector2` object in C# can be multiplied with a `float` object in Python.
The following code shows this usage.

```csharp
Vector2 a = new Vector2(1, 2);
Vector2 b = a * 2.0f;
Vector2 c = 2.0f * a;
```

As you can see, things are slightly different from `__add__`.
Because the `float` operand can be on the left or right side of the `*` operator.
In this case, you need to define `__mul__` and `__rmul__` at the same time.

```csharp
public class PyVector2Type: PyTypeObject{
    public override string Name => "Vector2";
    public override System.Type CSType => typeof(Vector2);

    // ...

    [PythonBinding]
    public object __mul__(Vector2 self, object other){
        if(!(other is float)) return VM.NotImplemented;
        return self * (float)other;
    }

    [PythonBinding]
    public object __rmul__(Vector2 self, object other){
        if(!(other is float)) return VM.NotImplemented;
        return self * (float)other;
    }
}
```

Finally, let's implement the constructor of `Vector2`.
`__new__` magic method must be defined.

```csharp
public class PyVector2Type: PyTypeObject{
    public override string Name => "Vector2";
    public override System.Type CSType => typeof(Vector2);

    [PythonBinding]
    public object __new__(PyTypeObject cls, params object[] args){
        if(args.Length == 0) return new Vector2();
        if(args.Length == 2){
            float x = vm.PyCast<float>(args[0]);
            float y = vm.PyCast<float>(args[1]);
            return new Vector2(x, y);
        }
        vm.TypeError("Vector2.__new__ takes 0 or 2 arguments");
        return null;
    }
}
```

Here we use `params object[] args` to tell the bindings that the constructor can take any number of arguments.
It is equivalent to `def __new__(cls, *args)` in Python.
Note that Python does not support method overloading.
So we manually check the number of arguments and their types to determine which constructor to call.

For fields, we can form a Python property by defining a getter and a setter.
By using `[PythonBinding(BindingType.Getter)]` and `[PythonBinding(BindingType.Setter)]` attributes.

!!!
However, this has certain limitations for value types. Because `Vector2` is a struct, it is passed by value.
So our setter will not be able to modify the original `Vector2` object.
!!!

```csharp
public class PyVector2Type: PyTypeObject{
    public override string Name => "Vector2";
    public override System.Type CSType => typeof(Vector2);

    [PythonBinding(BindingType.Getter)]
    public object x(Vector2 self) => self.x;

    [PythonBinding(BindingType.Setter)]
    public void x(Vector2 self, object value) => self.x = vm.PyCast<float>(value);

    [PythonBinding(BindingType.Getter)]
    public object y(Vector2 self) => self.y;

    [PythonBinding(BindingType.Setter)]
    public void y(Vector2 self, object value) => self.y = vm.PyCast<float>(value);
}
```

Once you have done all the above, you must register the type to the VM.
Here we set it into `builtins` module, so that it can be accessed from anywhere.

```csharp
vm.RegisterType(new PyVector2Type(), vm.builtins);
```

To summarize, manual static bindings provide detailed control for exposing a C# class to Python.
You decide which methods and variables to expose, and how to expose them.
This is our recommended way to define bindings. Also it is the most performant way.

### Automatic Static Bindings

Automatic static bindings use C# reflection to automatically generate bindings for a C# class.
It is convenient for testing and prototyping, but it is slow and unsafe since the user can access any member of the class.

```csharp
vm.RegisterAutoType<Vector2>(vm.builtins);
```

That's all you need to do. The `RegisterAutoType<T>` method will automatically generate bindings for `Vector2`.


## Dynamic Bindings

Dynamic bindings allow you to add a single C# lambda function to an object at runtime.

```csharp
delegate object NativeFuncC(VM vm, object[] args);
```

+ `CSharpLambda BindFunc(PyObject obj, string name, int argc, NativeFuncC f)`

It is similar to `bind_func` in [C++ API](../bindings/).
