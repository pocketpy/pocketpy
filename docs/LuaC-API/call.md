---
title: Callables
icon: dot
order: 6
---

#### `bool pkpy_call(pkpy_vm*, int argc)`

First push callable you want to call, then push the arguments to send.

+ `argc` is the number of arguments that was pushed (not counting the callable).

#### `bool pkpy_call_method(pkpy_vm*, const char* name, int argc)`

First push the object the method belongs to (self), then push the the argments.

+ `argc` is the number of arguments that was pushed (not counting the callable or self)
+ `name` is the name of the method to call on the object
