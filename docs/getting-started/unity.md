---
icon: code
label: Unity Engine
order: 4
---

## Introduction

PocketPy for Unity can be installed via Unity Asset Store.

https://assetstore.unity.com/packages/slug/241120



## Example 01

```csharp
using UnityEngine;

public class Test01 : MonoBehaviour
{
    // Start is called before the first frame update
    void Start()
    {
        // Create a virtual machine
        pkpy.VM vm = new pkpy.VM();

        // Create a list
        vm.exec("a = [1, 2, 3]");

        // Eval the sum of the list
        string result = vm.eval("sum(a)");
        Debug.Log(result);   // 6

        // Print to the standard output
        vm.exec("print(a)");
        pkpy.PyOutput o = vm.read_output();
        Debug.Log(o.stdout); // [1, 2, 3]

        // Create a binding
        vm.bind("builtins", "test", (double x) => x+1);  
        Debug.Log(vm.eval("test(3.14)")); // '4.14'
    }
}
```



## Example 02

```csharp
using UnityEngine;
using UnityEngine.UI;

public class Test02 : MonoBehaviour
{
    Text text;
    pkpy.VM vm;

    // Start is called before the first frame update
    void Start()
    {
        text = GetComponent<Text>();
        Application.targetFrameRate = 60;
        vm = new pkpy.VM();
        vm.exec("a = 0");
    }

    // Update is called once per frame
    void Update()
    {
        if(vm == null) return;
        vm.exec("a += 1");
        text.text = vm.eval("a");
    }
}
```

