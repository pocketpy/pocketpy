---
label: Examples
icon: dot
order: 4
---

See `Assets/PocketPython/Examples` after you import the plugin.


### Primes Example

```csharp
using UnityEngine;

namespace PocketPython
{

    /// <summary>
    /// Example of using PocketPython to find prime numbers.
    /// </summary>
    public class PrimesExample : MonoBehaviour
    {
        // Start is called before the first frame update
        void Start()
        {
            var vm = new VM();
            const string source = @"
def is_prime(x):
  if x < 2:
    return False
  for i in range(2, x):
    if x % i == 0:
      return False
  return True

primes = [i for i in range(2, 20) if is_prime(i)]
print(primes)
";
            CodeObject code = vm.Compile(source, "main.py", CompileMode.EXEC_MODE);
            vm.Exec(code);  // [2, 3, 5, 7, 11, 13, 17, 19]
        }
    }

}
```



### Vector2 Example

```csharp
using UnityEngine;

namespace PocketPython
{

    /// <summary>
    /// Example of making UnityEngine.Vector2 available to Python.
    /// </summary>
    public class Vector2Example : MonoBehaviour
    {
        // Start is called before the first frame update
        void Start()
        {
            var vm = new VM();
            // register UnityEngine.Vector2 type into the builtins module
            vm.RegisterAutoType<Vector2>(vm.builtins);

            vm.Exec("print(Vector2)", "main.py"); // <class 'Vector2'>
            vm.Exec("v = Vector2(1, 2)", "main.py");
            vm.Exec("print(v)", "main.py"); // (1.0, 2.0)
            vm.Exec("print(v.x)", "main.py"); // 1.0
            vm.Exec("print(v.y)", "main.py"); // 2.0
            vm.Exec("print(v.magnitude)", "main.py"); // 2.236068
            vm.Exec("print(v.normalized)", "main.py"); // (0.4472136, 0.8944272)
            vm.Exec("print(Vector2.Dot(v, v))", "main.py"); // 5.0
            vm.Exec("print(Vector2.get_up())", "main.py"); // (0.0, 1.0)

            Vector2 v = (Vector2)vm.Eval("Vector2(3, 4) + v");
            Debug.Log(v); // (4.0, 6.0)
        }
    }

}
```



### MyClass Example

```csharp
using UnityEngine;
using System;

namespace PocketPython
{

    public class MyClass
    {
        public string title;
        public string msg;

        public void Print()
        {
            Debug.Log(title + ": " + msg);
        }
    }

    public class PyMyclassType : PyTypeObject
    {
        public override string Name => "my_class";
        public override Type CSType => typeof(MyClass);

        [PythonBinding]
        public object __new__(PyTypeObject cls)
        {
            return new MyClass();
        }

        [PythonBinding(BindingType.Getter)]
        public string title(MyClass value) => value.title;

        [PythonBinding(BindingType.Getter)]
        public string msg(MyClass value) => value.msg;

        [PythonBinding(BindingType.Setter)]
        public void title(MyClass value, string title) => value.title = title;

        [PythonBinding(BindingType.Setter)]
        public void msg(MyClass value, string msg) => value.msg = msg;

        [PythonBinding]
        public void print(MyClass value) => value.Print();
    }


    /// <summary>
    /// Example of binding a custom C# class to Python.
    /// </summary>
    public class MyClassExample : MonoBehaviour
    {
        // Start is called before the first frame update
        void Start()
        {
            var vm = new VM();

            // register MyClass type into the builtins module
            vm.RegisterType(new PyMyclassType(), vm.builtins);

            vm.Exec("print(my_class)", "main.py"); // <class 'my_class'>
            vm.Exec("c = my_class()", "main.py");
            vm.Exec("c.title = 'Greeting'", "main.py");
            vm.Exec("c.msg = 'Hello, world!'", "main.py");

            string title = vm.Eval("c.title").ToString();
            string msg = vm.Eval("c.msg").ToString();

            Debug.Log(title + ": " + msg); // Greeting: Hello, world!

            vm.Exec("c.print()", "main.py"); // Greeting: Hello, world!
        }
    }

}
```

