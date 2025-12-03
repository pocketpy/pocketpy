# Binding Return Value Checker

## Overview

The `check_binding_retval.py` script is a static analysis tool that validates Python binding functions in the pocketpy codebase. It ensures that all functions bound to Python properly set return values before returning `true`.

## Purpose

In pocketpy's C API, when a binding function returns `true`, it indicates successful execution. According to the API conventions, the function MUST set a return value through one of these methods:

1. **Direct assignment** using `py_new*` functions:
   ```c
   py_newint(py_retval(), 42);
   return true;
   ```

2. **Setting None** explicitly:
   ```c
   py_newnone(py_retval());
   return true;
   ```

3. **Using assignment**:
   ```c
   py_assign(py_retval(), some_value);
   return true;
   ```

4. **Calling functions** that set `py_retval()` internally:
   ```c
   bool ok = py_import("module_name");  // Sets py_retval() on success
   if(ok) return true;
   ```

## Usage

### Basic Usage

Run the checker on default directories (`src/bindings` and `src/modules`):

```bash
python scripts/check_binding_retval.py
```

### Verbose Mode

Enable verbose output for debugging:

```bash
python scripts/check_binding_retval.py --verbose
```

### Custom Directories

Check specific directories:

```bash
python scripts/check_binding_retval.py --dirs src/bindings src/modules src/custom
```

## Exit Codes

- `0`: No issues found (success)
- `1`: Issues found (binding functions missing return value assignment)
- `2`: Script error (e.g., file access error)

## Integration

The checker is integrated into the CI/CD pipeline through the GitHub Actions workflow. It runs automatically on:

- Push events
- Pull request events

The check is part of the "Run Script Check" step in `.github/workflows/main.yml`.

## What It Checks

The tool analyzes all functions that are bound to Python through:

- `py_bindfunc()`
- `py_bind()`
- `py_bindmagic()`
- `py_bindmethod()`
- `py_bindproperty()`

For each bound function, it verifies that:

1. If the function contains `return true` statements
2. There are corresponding assignments to `py_retval()`
3. Comments are excluded from analysis (to avoid false positives)

## Functions That Set py_retval() Internally

The checker recognizes these functions that internally set `py_retval()`:

- `py_import()` - Sets py_retval() to the imported module on success
- `py_call()` - Sets py_retval() to the call result
- `py_iter()` - Sets py_retval() to the iterator
- `py_str()` - Sets py_retval() to string representation
- `py_repr()` - Sets py_retval() to repr string
- `py_getattr()` - Sets py_retval() to attribute value
- `py_next()` - Sets py_retval() to next value
- `py_getitem()` - Sets py_retval() to the item
- `py_vectorcall()` - Sets py_retval() to call result

## Example Issues

### Issue: Missing Return Value

```c
static bool my_function(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    // Do some work...
    return true;  // BUG: Should set py_retval() before returning!
}
```

**Fix:**

```c
static bool my_function(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    // Do some work...
    py_newnone(py_retval());  // Set return value to None
    return true;
}
```

### Correct: Using py_new* Functions

```c
static bool add_numbers(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_i64 a = py_toint(py_arg(0));
    py_i64 b = py_toint(py_arg(1));
    py_newint(py_retval(), a + b);  // Sets return value
    return true;
}
```

### Correct: Calling Functions That Set py_retval()

```c
static bool import_module(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    int res = py_import(py_tostr(py_arg(0)));  // Sets py_retval() on success
    if(res == -1) return false;
    if(res) return true;  // py_retval() already set by py_import
    return ImportError("module not found");
}
```

## False Positives

The checker may occasionally report false positives for:

1. **Helper functions** that are bound but shouldn't set return values (rare)
2. **Complex control flow** where the return value is set conditionally

If you encounter a false positive, verify that:
1. The function is actually a Python-bound callable
2. The return value is properly set in all code paths leading to `return true`

## Maintenance

When adding new patterns or functions that set `py_retval()` internally:

1. Update the `RETVAL_SETTING_FUNCTIONS` set in `check_binding_retval.py`
2. Add corresponding test cases
3. Update this documentation

## Related Documentation

- [pocketpy C API Documentation](https://pocketpy.dev/c-api/)
- [Binding Functions Guide](https://pocketpy.dev/c-api/bindings/)
