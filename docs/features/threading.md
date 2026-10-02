---
icon: dot
title: Compute Threads
order: 82
---

# Compute threads

A pocketpy VM contains independent modules, globals, and Python objects.
There are 16 VM slots, indexed `0` through `15`; VM 0 is the default.
**Only one native thread may access a particular VM at a time.**

Build with `PK_ENABLE_THREADS=ON` to use `pkpy.ComputeThread`. This is the
CMake default, but not the default for manual builds.
Check `pkpy.configmacros['PK_ENABLE_THREADS']` when a build lacks the class.

## Run a job in another VM

This example needs no external Python files:

```python
from pkpy import ComputeThread

worker = ComputeThread(1)
worker.exec('def square_sum(count):\n    return sum([i * i for i in range(count)])')

worker.submit_call('square_sum', 100)
# The main thread can do other work while the job runs.
worker.wait_for_done()

error = worker.last_error()
if error is not None:
    raise RuntimeError(error)

assert worker.last_retval() == 328350
```

`exec()` initializes the worker VM synchronously. `submit_call()` evaluates
the function name in that VM and starts a background job. A main-thread
function or global is not automatically visible in the worker.

## Job API

| Member | Behavior |
| --- | --- |
| `ComputeThread(vm_index)` | Reserve a worker VM from 1 through 15; use a distinct index per live worker. |
| `exec(source)`, `eval(source)` | Run synchronously in the worker VM. |
| `submit_exec(source)` | Submit statements; the successful result is `None`. |
| `submit_eval(source)` | Submit an expression. |
| `submit_call(expression, *args, **kwargs)` | Evaluate a callable in the worker and invoke it with copied arguments. |
| `is_done` | Whether the submitted job has finished. |
| `wait_for_done()` | Wait until completion. |
| `last_error()` | Formatted worker error, or `None` after success. Read only after completion. |
| `last_retval()` | Deserialize the result of a successful completed job. |

Only one job can run per worker at a time. Wait for completion before submitting
another job, and collect the result before a new job replaces it.

In a frame loop, poll `is_done` once per frame. Read `last_error()` before
`last_retval()`; a failed job has no usable return value. Keep the worker
object alive until completion, and finish all jobs before finalizing the host.

## Data and native resources

Arguments and results cross the VM boundary through [pickle](../modules/pickle.md).
They must be serializable; mutable containers are copied, not shared.
For class instances, the corresponding class must be importable in the worker.
Each VM loads its own Python module state.

Native libraries and host resources can still share process-wide state.
Protect that state in your bindings. Do not pass `py_Ref` values between VMs
or share a VM with a native worker that is already running.

Use `pkpy.currentvm()` to identify the current VM. Custom import and output
callbacks must be configured for each VM that needs them.
