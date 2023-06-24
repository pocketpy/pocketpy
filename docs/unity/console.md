---
label: Python console
icon: dot
order: 5
---

You can open the Python console in Unity by clicking the `Window/Python Console` menu item.

By default, the console creates a unmodified `VM` instance to execute your code.
You may want to provide an enhanced `VM` instance for the console in Unity Editor.
For example, adding some class bindings in `UnityEngine` namespace.

To do this, you need to create a class derived from `VM` and put it in `Assets/Editor/` folder.
By adding `[EditorVM]` attribute to the class,
the console will use it instead of the default `VM` instance.


```csharp
using UnityEngine;
using PocketPython;

[EditorVM]      // this attribute is required
public class EnhancedVM: VM{
    public EnhancedVM() {
        RegisterAutoType<GameObject>(builtins);
        RegisterAutoType<Transform>(builtins);
        RegisterAutoType<Vector2>(builtins);
        RegisterAutoType<Vector3>(builtins);
        // ...
    }
}
```